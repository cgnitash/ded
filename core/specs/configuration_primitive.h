
#pragma once

#include <cassert>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../utilities/tmp.h"

namespace ded
{
namespace specs
{

class ConfigurationPrimitive
{
private:
  std::variant<std::monostate, long, double, bool, std::string> value_;

  std::regex r_long{ R"~~(^\d+$)~~" };
  std::regex r_double{ R"~~(^\d+\.\d*$)~~" };
  std::regex r_bool{ R"~~(^(true)|(false)$)~~" };
  std::regex r_string{ R"~~(^".*"$)~~" };
  std::regex r_null{ R"~~(^NULL$)~~" };

public:
  template <typename T>
  void
      set_value(T value)
  {
    assert(std::holds_alternative<std::monostate>(value_) ||
           std::holds_alternative<T>(value_));
    value_ = value;
  }

  template <typename T>
  void
      get_value(T &value)
  {
    assert(std::holds_alternative<T>(value_));
    value = std::get<T>(value_);
  }

  void        parse(std::string);
  std::string value_as_string() const;
  std::string type_as_string() const;
};
}   // namespace specs
}   // namespace ded
