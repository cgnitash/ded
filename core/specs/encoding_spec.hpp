
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../language/parser.hpp"
#include "../utilities/term_colours.hpp"
#include "../utilities/utilities.hpp"
#include "configuration_primitive.hpp"
#include "signal_spec.hpp"

namespace ded
{

namespace specs
{

class EncodingSpec
{
  std::string                                   name_;
//  std::map<std::string, ConfigurationPrimitive> parameters_;
  Parameters                                              parameters_;

public:
  EncodingSpec(std::string name = "") : name_(name)
  {
  }
  EncodingSpec( language::Block);

  auto
      name() const
  {
    return name_;
  }

  template <typename ArgumentType>
  void
      bindParameter(std::string                           name,
                    ArgumentType                          value,
                    std::vector<Constraint<ArgumentType>> cons = {})
  {
	parameters_.bind(name, value, cons);
	  /*
    if (parameters_.find(name) != parameters_.end())
    {
      std::cout << "User error: parameter " << name
                << " has already been declared\n";
      throw SpecError{};
    }
    parameters_[name].setValue(value);
    parameters_[name].setConstraints(cons);
  */
  }

  template <typename T>
  void
      configureParameter(std::string name, T &value)
  {
    parameters_.configure(name, value); 
	  /*
    if (parameters_.find(name) == parameters_.end())
    {
      std::cout << "User error: parameter " << name
                << " has not been declared\n";
      throw SpecError{};
    }
    parameters_[name].get_value(value);
 */
  }


  // friend std::ostream &operator<<(std::ostream &out, SubstrateSpec e)
  std::vector<std::string> serialise(long) const;
  EncodingSpec  deserialise(std::vector<std::string> );
  std::string prettyPrint();

};

}   // namespace specs
}   // namespace ded
