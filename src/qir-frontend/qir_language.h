#pragma once

#include <util/language.h>

class qir_languaget : public languaget
{
public:
  bool parse(const std::string &path) override;

  bool final(contextt &context) override;

  bool typecheck(contextt &context, const std::string &module) override;

  std::string id() const override
  {
    return "qir";
  }

  void show_parse(std::ostream &out) override;

  bool from_expr(
    const exprt &expr,
    std::string &code,
    const namespacet &ns,
    unsigned flags) override;

  bool from_type(
    const typet &type,
    std::string &code,
    const namespacet &ns,
    unsigned flags) override;

  unsigned default_flags(presentationt target) const override;

  languaget *new_language() const override
  {
    return new qir_languaget;
  }
};

languaget *new_qir_language();
