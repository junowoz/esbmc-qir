#include <qir-frontend/qir_language.h>

#include <util/c_expr2string.h>
#include <util/message.h>

languaget *new_qir_language()
{
  return new qir_languaget;
}

bool qir_languaget::parse(const std::string &path)
{
  log_status("QIR frontend: parse not yet implemented for {}", path);
  return true;
}

bool qir_languaget::typecheck(contextt &, const std::string &)
{
  log_status("QIR frontend: typecheck not yet implemented");
  return true;
}

bool qir_languaget::final(contextt &)
{
  return false;
}

void qir_languaget::show_parse(std::ostream &out)
{
  out << "QIR frontend: show_parse not yet implemented\n";
}

bool qir_languaget::from_expr(
  const exprt &expr,
  std::string &code,
  const namespacet &ns,
  unsigned flags)
{
  code = c_expr2string(expr, ns, flags);
  return false;
}

bool qir_languaget::from_type(
  const typet &type,
  std::string &code,
  const namespacet &ns,
  unsigned flags)
{
  code = c_type2string(type, ns, flags);
  return false;
}

unsigned qir_languaget::default_flags(presentationt) const
{
  return 0;
}
