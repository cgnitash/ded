
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
      parameter(std::string                           name,
                ArgumentType &                        value,
                std::vector<Constraint<ArgumentType>> cons = {})
  {
    if (isConfigurable)
      parameters_.configure(name, value);
    else
      parameters_.bind(name, value, cons);
  }

  void
      nestedSubstrate(SubstrateSpec &e)
  {
    if (isConfigurable)
      e = es_;
    else
      es_ = e;
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

  void setConfigured(bool conf)
  {
	  isConfigurable = conf;
  }

  //  friend std::ostream &operator<<(std::ostream &out, PopulationSpec e)

  friend class concepts::Population;

private:
  bool          isConfigurable = true;
  std::string   name_;
  Parameters    parameters_;
  SubstrateSpec es_{ "null_entity" };
};

}   // namespace specs
}   // namespace ded
