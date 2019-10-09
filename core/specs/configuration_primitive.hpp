
#pragma once

#include <cassert>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../utilities/tmp.hpp"

namespace ded
{
namespace specs
{

struct SpecError
{
};

class ConfigurationPrimitive
{
private:
  std::variant<std::monostate, long, double, bool, std::string>    value_;
  std::vector<std::pair<std::function<bool(long)>, std::string>>   long_cons_;
  std::vector<std::pair<std::function<bool(double)>, std::string>> double_cons_;
  std::vector<std::pair<std::function<bool(bool)>, std::string>>   bool_cons_;
  std::vector<std::pair<std::function<bool(std::string)>, std::string>>
      string_cons_;

  std::regex r_long{ R"~~(^\d+$)~~" };
  std::regex r_double{ R"~~(^\d+\.\d*$)~~" };
  std::regex r_bool{ R"~~(^(true)|(false)$)~~" };
  std::regex r_string{ R"~~(^".*"$)~~" };
  std::regex r_null{ R"~~(^NULL$)~~" };

public:
  template <typename T>
  void
      setValue(T value)
  {
    assert(std::holds_alternative<std::monostate>(value_) ||
           std::holds_alternative<T>(value_));
    value_ = value;
  }

  void
      setConstraints(
          std::vector<std::pair<std::function<bool(long)>, std::string>> cons)
  {
    assert(std::holds_alternative<long>(value_));
    long_cons_ = cons;
  }

  void
      setConstraints(
          std::vector<std::pair<std::function<bool(double)>, std::string>> cons)
  {
    assert(std::holds_alternative<double>(value_));
    double_cons_ = cons;
  }

  void
      setConstraints(
          std::vector<std::pair<std::function<bool(bool)>, std::string>> cons)
  {
    assert(std::holds_alternative<bool>(value_));
    bool_cons_ = cons;
  }

  void
      setConstraints(
          std::vector<std::pair<std::function<bool(std::string)>, std::string>> cons)
  {
    assert(std::holds_alternative<std::string>(value_));
    string_cons_ = cons;
  }

  std::optional<std::string>
      checkConstraints() const
  {

    return std::visit(
        utilities::TMP::overload_set{
            [](std::monostate) ->  std::optional<std::string>{ return {}; },
            [this](long v) -> std::optional<std::string> {
              for (auto c : long_cons_)
                if (!c.first(v))
                  return c.second;
              return {};
            },
            [this](double v) -> std::optional<std::string> {
              for (auto c : double_cons_)
                if (!c.first(v))
                  return c.second;
              return {};
            },
            [this](bool v) -> std::optional<std::string> {
              for (auto c : bool_cons_)
                if (!c.first(v))
                  return c.second;
              return {};
            },
            [this](std::string v) -> std::optional<std::string> {
              for (auto c : string_cons_)
                if (!c.first(v))
                  return c.second;
              return {};
            } },
        value_);
  }

  template <typename T>
  void
      get_value(T &value)
  {
    assert(std::holds_alternative<T>(value_));
    value = std::get<T>(value_);
  }

  void        parse(std::string);
  std::string valueAsString() const;
  std::string typeAsString() const;
};
}   // namespace specs
}   // namespace ded
