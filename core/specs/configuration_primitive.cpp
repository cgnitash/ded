
#include "configuration_primitive.h"

namespace ded {
	namespace specs {
  std::string ConfigurationPrimitive::value_as_string() const
  {
    std::string s;
    std::visit(
        utilities::TMP::overload_set{
            [&](std::monostate) { s = "NULL"; },
            [&](long v) { s = std::to_string(v); },
            [&](double v) { s = std::to_string(v); },
            [&](bool v) { s = v ? "true" : "false"; },
            [&](std::string v) { s = "\"" + v + "\""; } },
        value_);
    return s;
  }

  std::string ConfigurationPrimitive::type_as_string() const
  {
    std::string s;
    std::visit(
        utilities::TMP::overload_set{
            [&](std::monostate) { s = "NULL"; },
            [&](long) { s = "long"; },
            [&](double) { s = "double"; },
            [&](bool) { s = "bool"; },
            [&](std::string) { s = "string"; } },
        value_);
    return s;
  }
}
}
