
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../language/parser.hpp"
#include "../utilities/term_colours.hpp"
#include "configuration_primitive.hpp"
#include "substrate_spec.hpp"

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
  std::string   name_;
  Parameters    parameters_;
  SubstrateSpec es_{ "null_entity" };

public:
  PopulationSpec(std::string name = "") : name_(name)
  {
  }

  PopulationSpec(language::Block);

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

  template <typename ArgumentType>
  void
      bindParameter(std::string                           name,
                    ArgumentType                          value,
                    std::vector<Constraint<ArgumentType>> cons = {})
  {
    parameters_.bind(name, value, cons);
  }

  template <typename T>
  void
      configureParameter(std::string name, T &value)
  {
    parameters_.configure(name, value);
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

  std::vector<std::string> serialise(long) const;
  PopulationSpec           deserialise(std::vector<std::string>);
  std::string              prettyPrint();

  SubstrateSpec
      instantiateNestedSubstrateUserParameterSizes()
  {
    es_.instantiateUserParameterSizes();
    return es_;
  }
  //  friend std::ostream &operator<<(std::ostream &out, PopulationSpec e)

  friend class concepts::Population;
};

}   // namespace specs
}   // namespace ded
