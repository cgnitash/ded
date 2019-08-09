
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <range/v3/all.hpp>
#include <regex>
#include <string>
#include <variant>

#include "../language/parser.h"
#include "../utilities/term_colours.h"
#include "configuration_primitive.h"
#include "signal_spec.h"

namespace ded
{

// forward declaration to provide friendship
namespace concepts
{
class Entity;
}

namespace specs
{

class EntitySpec
{
  struct NestedSpec
  {
    std::unique_ptr<EntitySpec> e;
    NestedSpec() = default;
    NestedSpec(const NestedSpec &ns) : e(std::make_unique<EntitySpec>(*ns.e))
    {
    }
  };

  std::string                                   name_;
  std::map<std::string, ConfigurationPrimitive> parameters_;
  std::map<std::string, NestedSpec>             nested_;
  IO                                            io_;

public:
  EntitySpec(std::string name = "") : name_(name)
  {
  }
  EntitySpec(language::Parser, language::Block);

  auto
      name() const
  {
    return name_;
  }

  template <typename T>
  void
      bindParameter(
          std::string                                                 name,
          T                                                           value,
          std::vector<std::pair<std::function<bool(T)>, std::string>> cons = {})

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

  template <typename T>
  void
      configureParameter(std::string name, T &value)
  {
    if (parameters_.find(name) == parameters_.end())
    {
      std::cout << "User error: parameter " << name
                << " has not been declared\n";
      throw SpecError{};
    }
    parameters_[name].get_value(value);
  }

  void
      bindIinput(std::string name, std::string value)
  {
    io_.inputs_.push_back({ name, SignalSpec{ name, name, value } });
  }

  void
      configureInput(std::string name, std::string &value)
  {
    auto i =
        ranges::find_if(io_.inputs_, [=](auto ns) { return ns.first == name; });
    value = i->second.identifier();
  }

  void
      bindOutput(std::string name, std::string value)
  {
    io_.outputs_.push_back({ name, SignalSpec{ name, name, value } });
  }

  void
      configureOutput(std::string name, std::string &value)
  {
    auto i = ranges::find_if(io_.outputs_,
                             [=](auto ns) { return ns.first == name; });
    value  = i->second.identifier();
  }

  void
      bindEntity(std::string name, EntitySpec env)
  {
    nested_[name].e = std::make_unique<EntitySpec>(env);
  }

  void
      configureEntity(std::string name, EntitySpec &e)
  {
    if (!nested_[name].e)
    {
      std::cout << "Warning: <" << name_ << ":" << name
                << "> environment spec has not been bind-ed (probably error)\n";
      //      std::exit(1);
    }
    else
      e = *nested_[name].e;
  }

  // friend std::ostream &operator<<(std::ostream &out, EntitySpec e)
  std::string dump(long depth) const;
  EntitySpec  parse(std::vector<std::string> pop_dump);
  std::string prettyPrint();

  IO instantiateUserParameterSizes(int);

  friend class concepts::Entity;
};

}   // namespace specs
}   // namespace ded
