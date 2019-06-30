
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
      configure_parameter(std::string name, T &value)
  {
    parameters_[name].get_value(value);
  }

  template <typename T>
  void
      bind_parameter(std::string name, T value)
  {
    parameters_[name].set_value(value);
  }

  void
      bind_input(std::string name, std::string value)
  {
    io_.inputs_.push_back({ name, SignalSpec{ name, name, value } });
  }

  void
      configure_input(std::string name, std::string &value)
  {
    auto i =
        ranges::find_if(io_.inputs_, [=](auto ns) { return ns.first == name; });
    value = i->second.identifier();
  }

  void
      bind_output(std::string name, std::string value)
  {
    io_.outputs_.push_back({ name, SignalSpec{ name, name, value } });
  }

  void
      configure_output(std::string name, std::string &value)
  {
    auto i = ranges::find_if(io_.outputs_,
                             [=](auto ns) { return ns.first == name; });
    value  = i->second.identifier();
  }

  void
      bind_entity(std::string name, EntitySpec env)
  {
    nested_[name].e = std::make_unique<EntitySpec>(env);
  }

  void
      configure_entity(std::string name, EntitySpec &e)
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
  std::string dump(long depth);
  EntitySpec  parse(std::vector<std::string> pop_dump);
  std::string pretty_print();

  IO instantiate_user_parameter_sizes(int);

  friend class concepts::Entity;
};

}   // namespace specs
}   // namespace ded
