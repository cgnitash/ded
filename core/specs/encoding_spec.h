
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../language/parser.h"
#include "../utilities/term_colours.h"
#include "../utilities/utilities.h"
#include "configuration_primitive.h"
#include "signal_spec.h"

namespace ded
{

namespace specs
{

class EncodingSpec
{
  std::string                                   name_;
  std::map<std::string, ConfigurationPrimitive> parameters_;

public:
  EncodingSpec(std::string name = "") : name_(name)
  {
  }
  EncodingSpec(language::Parser, language::Block);

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


  // friend std::ostream &operator<<(std::ostream &out, SubstrateSpec e)
  std::vector<std::string> serialise(long) const;
  EncodingSpec  deserialise(std::vector<std::string> );
  std::string prettyPrint();

};

}   // namespace specs
}   // namespace ded
