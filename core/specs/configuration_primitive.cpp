
#include "configuration_primitive.hpp"
#include "../utilities/utilities.hpp"

namespace ded
{
namespace specs
{

void
    ConfigurationPrimitive::parse(std::string s)
{
  std::smatch m;
  if (std::regex_match(s, m, r_long))
  {
    value_ = std::stol(m.str());
    return;
  }
  if (std::regex_match(s, m, r_double))
  {
    value_ = std::stod(m.str());
    return;
  }
  if (std::regex_match(s, m, r_bool))
  {
    bool b{};
    std::istringstream{ m.str() } >> std::boolalpha >> b;
    value_ = b;
    return;
  }
  if (std::regex_match(s, m, r_string))
  {
    std::string s = m.str();
    value_        = s.substr(1, s.length() - 2);
    return;
  }
  return;
}

std::string
    ConfigurationPrimitive::valueAsString() const
{
  return std::visit(
      utilities::TMP::overload_set{
          [](std::monostate) -> std::string { return "NULL"; },
          [](long v) -> std::string { return std::to_string(v); },
          [](double v) -> std::string { return std::to_string(v); },
          [](bool v) -> std::string { return v ? "true" : "false"; },
          [](std::string v) -> std::string { return "\"" + v + "\""; } },
      value_);
}

std::string
    ConfigurationPrimitive::typeAsString() const
{
  return std::visit(
      utilities::TMP::overload_set{
          [](std::monostate) -> std::string { return "NULL"; },
          [](long) -> std::string { return "long"; },
          [](double) -> std::string { return "double"; },
          [](bool) -> std::string { return "bool"; },
          [](std::string) -> std::string { return "string"; } },
      value_);
}

void
    Parameters::loadFromSpec(
        std::vector<language::TokenAssignment> const &overrides,
        std::string                                   component_name)
{
  for (auto over : overrides)
  {
    auto name  = over.lhs_;
    auto value = over.rhs_;

    auto f = rs::find_if(parameters_,
                         [&](auto param) { return param.first == name.expr_; });
    if (f == parameters_.end())
    {
      errInvalidToken(name,
                      "this does not override any parameters of " + component_name,
                      parameters_ | rv::keys |
                          rs::to<std::vector<std::string>>);
      throw language::ParserError{};
    }

    ConfigurationPrimitive cp;
    cp.parse(value.expr_);
    if (cp.typeAsString() != f->second.typeAsString())
    {
      errInvalidToken(value,
                      "type mismatch, should be " + f->second.typeAsString());
      throw language::ParserError{};
    }
    f->second.parse(cp.valueAsString());
    auto con = f->second.checkConstraints();
    if (con)
    {
      errInvalidToken(value, "parameter constraint not satisfied\n" + *con);
      throw language::ParserError{};
    }
  }
}
}   // namespace specs
}   // namespace ded
