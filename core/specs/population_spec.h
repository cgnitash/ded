
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../language/parser.h"
#include "../utilities/term_colours.h"
#include "configuration_primitive.h"
#include "substrate_spec.h"

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
  SubstrateSpec es_{ "null_entity" };

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
      bindSubstrate(SubstrateSpec e)
  {
    es_ = e;
  }

  void
      configureSubstrate(SubstrateSpec &e)
  {
    e = es_;
  }

   std::vector<std::string>  serialise(long )const;
  PopulationSpec deserialise(std::vector<std::string> );
  std::string    prettyPrint();

  SubstrateSpec
      instantiateNestedSubstrateUserParameterSizes()
  {
	es_.instantiateUserParameterSizes(0);
	es_.checkNestedIO();
    return es_;
  }
  //  friend std::ostream &operator<<(std::ostream &out, PopulationSpec e)

  friend class concepts::Population;
};

}   // namespace specs
}   // namespace ded
