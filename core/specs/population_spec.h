
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
#include "entity_spec.h"

namespace ded
{

// forward declaration to provide friendship
namespace concepts
{
class Population;
}

namespace specs
{

class PopulationSpec
{
  std::string                                   name_;
  std::map<std::string, ConfigurationPrimitive> parameters_;
  // std::map<std::string, std::string>             inputs_;
  // std::map<std::string, std::string>             outputs_;
  EntitySpec es_{ "null_entity" };

public:
  // PopulationSpec() = default;

  PopulationSpec(std::string name = "") : name_(name)
  {
  }
  PopulationSpec(language::Parser, language::Block);

  auto
      name() const
  {
    return name_;
  }

  auto
      parameters() const
  {
    return parameters_;
  }

  // auto inputs() const { return inputs_; }

  // auto outputs() const { return outputs_; }

  template <typename T>
  void
      bind_parameter(
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
    parameters_[name].set_value(value);
	parameters_[name].set_constraints(cons);

  }

  template <typename T>
  void
      configure_parameter(std::string name, T &value)
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
      bind_entity(EntitySpec e)
  {
    es_ = e;
  }

  void
      configure_entity(EntitySpec &e)
  {
    e = es_;
  }

  std::string    dump(long depth)const;
  PopulationSpec parse(std::vector<std::string> pop_dump);
  std::string    pretty_print();

  IO
      instantiate_nested_entity_user_parameter_sizes()
  {
    return es_.instantiate_user_parameter_sizes(0);
  }
  //  friend std::ostream &operator<<(std::ostream &out, PopulationSpec e)

  friend class concepts::Population;
};

}   // namespace specs
}   // namespace ded
