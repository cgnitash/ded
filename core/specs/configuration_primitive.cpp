
#include "configuration_primitive.hpp"

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
}   // namespace specs
}   // namespace ded
