
#pragma once

#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>
#include <cassert>

#include "../enhanced_type_traits.h"

class configuration_primitive {
private:

  std::variant<std::monostate, long, double, bool, std::string> value_;

  std::regex r_long{ R"~~(^\d+$)~~" };
  std::regex r_double{ R"~~(^\d+\.\d*$)~~" };
  std::regex r_bool{ R"~~(^(true)|(false)$)~~" };
  std::regex r_string{ R"~~(^"[^"]*"$)~~" };
  std::regex r_null{ R"~~(^NULL$)~~" };

public:

  template <typename T> void set_value(T value)
  {
    assert(std::holds_alternative<std::monostate>(value_) ||
           std::holds_alternative<T>(value_));
    value_ = value;
  }

  template <typename T> void get_value(T &value)
  {
    assert(std::holds_alternative<T>(value_));
    value = std::get<T>(value_);
  }

  void parse(std::string s)
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
      value_        = s.substr(1, s.length() - 1);
      return;
    }
    std::cout << "ERROR: unable to parse configuration primitive " << s
              << std::endl;
    std::exit(1);
  }

  std::string value_as_string() const
  {
    std::string s;
    std::visit(
        enhanced_type_traits::overload_set{
            [&](std::monostate) { s = "NULL"; },
            [&](long v) { s = std::to_string(v); },
            [&](double v) { s = std::to_string(v); },
            [&](bool v) { s = v ? "true" : "false"; },
            [&](std::string v) { s = v; } },
        value_);
    return s;
  }

  std::string type_as_string() const
  {
    std::string s;
    std::visit(
        enhanced_type_traits::overload_set{
            [&](std::monostate) { s = "NULL"; },
            [&](long) { s = "long"; },
            [&](double) { s = "double"; },
            [&](bool) { s = "bool"; },
            [&](std::string) { s = "string"; } },
        value_);
    return s;
  }
};

