
#pragma once

#include <cassert>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../utilities/tmp.hpp"
#include "../language/token.hpp"

namespace ded
{
namespace specs
{

struct SpecError
{
};

inline static const std::regex r_long{ R"~~(^\d+$)~~" };
inline static const std::regex r_double{ R"~~(^\d+\.\d*$)~~" };
inline static const std::regex r_bool{ R"~~(^(true)|(false)$)~~" };
inline static const std::regex r_string{ R"~~(^".*"$)~~" };
inline static const std::regex r_null{ R"~~(^NULL$)~~" };

template <typename ArgumentType>
struct Constraint
{
  std::function<bool(ArgumentType)> function_{};
  std::string                   value_;
};

class ConfigurationPrimitive
{
private:
  std::variant<std::monostate, long, double, bool, std::string> value_;
  std::vector<Constraint<long>>                                 long_cons_;
  std::vector<Constraint<double>>                               double_cons_;
  std::vector<Constraint<bool>>                                 bool_cons_;
  std::vector<Constraint<std::string>>                          string_cons_;

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
      setConstraints(std::vector<Constraint<long>> cons)
  {
    assert(std::holds_alternative<long>(value_));
    long_cons_ = cons;
  }

  void
      setConstraints(std::vector<Constraint<double>> cons)
  {
    assert(std::holds_alternative<double>(value_));
    double_cons_ = cons;
  }

  void
      setConstraints(std::vector<Constraint<bool>> cons)
  {
    assert(std::holds_alternative<bool>(value_));
    bool_cons_ = cons;
  }

  void
      setConstraints(std::vector<Constraint<std::string>> cons)
  {
    assert(std::holds_alternative<std::string>(value_));
    string_cons_ = cons;
  }

  std::optional<std::string>
      checkConstraints() const
  {

    return std::visit(
        utilities::TMP::overload_set{
            [](std::monostate) -> std::optional<std::string> { return {}; },
            [this](long v) -> std::optional<std::string> {
              for (auto c : long_cons_)
                if (!c.function_(v))
                  return c.value_;
              return {};
            },
            [this](double v) -> std::optional<std::string> {
              for (auto c : double_cons_)
                if (!c.function_(v))
                  return c.value_;
              return {};
            },
            [this](bool v) -> std::optional<std::string> {
              for (auto c : bool_cons_)
                if (!c.function_(v))
                  return c.value_;
              return {};
            },
            [this](std::string v) -> std::optional<std::string> {
              for (auto c : string_cons_)
                if (!c.function_(v))
                  return c.value_;
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

struct Parameters
{
  std::map<std::string, ConfigurationPrimitive>           parameters_;

  template <typename ArgumentType>
  void
      bind(std::string                           name,
                    ArgumentType                          value,
                    std::vector<Constraint<ArgumentType>> cons = {})
  {
    if (parameters_.find(name) != parameters_.end())
    {
      std::cout << "User error: parameter " << name
                << " has already been declared\n";
      throw SpecError{};
    }
    parameters_[name].setValue(value);
    parameters_[name].setConstraints(cons);
  }
	  
  template <typename ArgumentType>
  void
      configure(std::string name, ArgumentType &value)
  {
    if (parameters_.find(name) == parameters_.end())
    {
      std::cout << "User error: parameter " << name
                << " has not been declared\n";
      throw SpecError{};
    }
    parameters_[name].get_value(value);
  }

  void loadFromSpec(std::vector<language::TokenAssignment> const &overrides,
                    std::string                                   component_name);
};

}   // namespace specs
}   // namespace ded
