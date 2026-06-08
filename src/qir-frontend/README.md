# ESBMC-QIR

> A QIR front-end for SMT-based bounded model checking in ESBMC.
>
> Status: Phase 0 scaffold implemented. Real QIR parsing, QIR-to-GOTO
> conversion, operational models, and regression tests are still future work.

## Overview

ESBMC-QIR is a planned Quantum Intermediate Representation front-end for
ESBMC. The goal is to make ESBMC consume QIR modules and lower them into
ESBMC's internal GOTO representation. From that point onward, the existing
ESBMC pipeline is reused: symbolic execution, SSA generation, SMT encoding,
and solving with the configured SMT backend.

The contribution is therefore not a new solver and not a new SMT encoder. It
is a new front-end/lowering layer:

```text
quantum source (Q#, Qiskit through external QIR-lowering paths, ...)
        │
        ▼
language compiler / lowering tool (qsc, qbraid-qir, qsharp interop, ...)
        │
        ▼
QIR / LLVM bitcode with __quantum__* intrinsics
        │
        ▼
qir-frontend
        │
        ▼
GOTO program
        │
        ▼
goto-symex
        │
        ▼
SSA
        │
        ▼
SMT formula
        │
        ▼
SMT solver
        │
        ▼
SAT   = counterexample within bound K
UNSAT = no violation within bound K
```

## Current implementation

The current branch implements the Phase 0 skeleton:

- `-DENABLE_QIR_FRONTEND=On` CMake option.
- Optional `qirfrontend` target.
- `QIR` language mode in `langapi`.
- `.qir` extension dispatch.
- Minimal `qir_languaget` implementation.

The current `parse()` and `typecheck()` methods intentionally return a clean
failure path with a `not yet implemented` diagnostic. They do not yet parse
LLVM IR, construct a symbol table, emit GOTO, or verify QIR properties.

## Why QIR and not Q# directly

Targeting QIR is the architectural choice because QIR is the LLVM-based
intermediate representation for hybrid quantum/classical programs. A Q#-only
front-end would require reimplementing Q# parsing and type checking, would be
sensitive to source-level language changes, and would not help programs that
come from other quantum front-ends.

By consuming QIR, ESBMC-QIR can target modules produced by compatible quantum
front-ends, subject to the supported QIR profile and intrinsic subset. This is
analogous to the role of Jimple in the ESBMC Java/Kotlin front-end: ESBMC
consumes a shared intermediate representation rather than duplicating source
front-ends.

QIR is not necessarily the final code executed by a QPU. Providers may lower it
further into target-specific representations. The point of ESBMC-QIR is to
verify the backend-facing IR level where allocation, release, reset,
measurement, result handling, and profile constraints are explicit.

## Thesis scope: qubit lifecycle safety

The first research slice is intentionally narrow: qubit lifecycle safety in QIR
programs.

Phase 1 targets a minimal structural subset spanning QIR Base Profile programs
plus a small Adaptive_RI fragment needed to model measurement-driven classical
control. Full adaptive runtime computation, dynamic integer computation,
probabilistic reasoning, and unitary equivalence checking are deferred.

The initial target properties are:

- release only clean qubits;
- no use after release;
- no double release;
- bounded number of allocated qubits;
- valid measurement-driven classical control flow.

The gate set exists only to make programs non-trivial. Proving algebraic gate
semantics such as `X * X = I` or full circuit equivalence is out of scope for
Phase 1.

## Target QIR intrinsic subset

| Family | Examples |
| --- | --- |
| Qubit allocation / release | `__quantum__rt__qubit_allocate`, `__quantum__rt__qubit_allocate_array`, `__quantum__rt__qubit_release`, `__quantum__rt__qubit_release_array` |
| Reset | `__quantum__qis__reset__body` |
| Measurement | `__quantum__qis__m__body`, `__quantum__qis__mz__body` |
| Single-qubit gates | `__quantum__qis__h__body`, `__quantum__qis__x__body`, `__quantum__qis__y__body`, `__quantum__qis__z__body` |
| Rotations | `__quantum__qis__rx__body`, `__quantum__qis__ry__body`, `__quantum__qis__rz__body` |
| Multi-qubit gates | `__quantum__qis__cnot__body`, `__quantum__qis__cz__body`, `__quantum__qis__swap__body` |
| Result management | `__quantum__rt__result_get_zero`, `__quantum__rt__result_get_one`, `__quantum__rt__result_equal`, `__quantum__rt__read_result` |
| Classical control on `Result` | Branches guarded by `__quantum__rt__result_equal` |

The exact accepted signatures must be pinned against the selected QIR producer
and profile before implementing the parser/converter.

## Typestate model

Phase 1 models each qubit as a deterministic typestate automaton:

```text
unallocated --alloc--> clean --use--> dirty
clean       --measure--> measured
dirty       --measure--> measured
dirty       --reset--> clean
measured    --reset--> clean
clean       --release--> released
released    --any op--> error
dirty       --release--> error
measured    --release--> error
```

`clean` means that the qubit is allocated and safe to release. Gates and
measurements make the qubit dirty or measured. `reset` restores the clean
state. This abstraction intentionally does not model amplitudes or probability.
Measurement results are modeled as nondeterministic `Zero`/`One` values so that
symbolic execution explores all measurement-driven paths.

## Expected ESBMC encoding

The intended lowering is property-directed:

- represent each QIR qubit as an abstract resource identifier;
- maintain ghost state for each qubit lifecycle state;
- translate QIR runtime/QIS intrinsics into GOTO statements and assertions;
- encode lifecycle violations as `__ESBMC_assert` failures;
- let ESBMC's existing goto-symex and SMT backends discharge the resulting VCs.

This abstraction cannot prove gate-level functional correctness. It is designed
to preserve the behaviours relevant to the monitored lifecycle properties
within the chosen QIR subset and ESBMC bound.

## Build

```sh
cmake -GNinja -Bbuild -S . \
  -DDOWNLOAD_DEPENDENCIES=On \
  -DENABLE_QIR_FRONTEND=On \
  -DENABLE_Z3=On \
  -DBUILD_TESTING=On \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo

ninja -C build esbmc
```

At this stage, invoking ESBMC on `.qir` input is expected to reach the stub and
report that QIR parsing is not yet implemented.

## Roadmap

1. Phase 0: build-system and language-mode skeleton.
2. Phase 1: parse a strict QIR subset and build the symbol table.
3. Phase 2: lower allocation, reset, release, measurement, and result branches
   into GOTO.
4. Phase 3: implement lifecycle assertions and regression tests.
5. Phase 4: add small Q#-generated QIR benchmarks.
6. Phase 5: evaluate scalability and document supported profiles.

## Initial regression targets

Planned examples:

- safe allocate/reset/release;
- missing reset before release;
- use after release;
- double release;
- measurement branch with both outcomes explored;
- qubit allocation bound violation.

## References

- ESBMC repository and architecture documentation.
- QIR Alliance specification and profile documentation.
- LLVM Language Reference Manual.
- QDK/Q# compiler documentation for QIR generation and profiles.
- Literature on SMT-based bounded model checking, typestate/resource-protocol
  verification, and quantum-program verification.
