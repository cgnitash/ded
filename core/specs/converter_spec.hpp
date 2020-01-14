
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
#include "signal_spec.hpp"

namespace ded
{

// forward declaration to provide friendship
namespace concepts
{
class Converter;
}

namespace specs
{

class ConverterSpec
{
  std::string                                   name_;
  //std::string                                   from_signal_;
  //std::string                                   to_signal_;
  std::map<std::string, ConfigurationPrimitive> parameters_;
  std::pair<SignalSpec,SignalSpec> args_; 
  // std::map<std::string, std::string>             inputs_;
  // std::map<std::string, std::string>             outputs_;
  //SubstrateSpec es_{ "null_entity" };

public:
  // ConverterSpec() = default;

  ConverterSpec(std::string name = "") : name_(name)
  {
  }

  ConverterSpec( language::Block);

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

  void bindFrom(std::string from)
  {
	args_.first = SignalSpec{"","",from};
  }

  void bindTo(std::string to)
  {
	args_.second = SignalSpec{"","",to};
  }
  /*
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
	*/

   std::vector<std::string>  serialise(long )const;
  ConverterSpec deserialise(std::vector<std::string> );
  std::string    prettyPrint();

  /*
  SubstrateSpec
      instantiateNestedSubstrateUserParameterSizes()
  {
	es_.instantiateUserParameterSizes(0);
	es_.checkNestedIO();
    return es_;
  }
  */
  //  friend std::ostream &operator<<(std::ostream &out, ConverterSpec e)

  friend class concepts::Converter;
};

}   // namespace specs
}   // namespace ded
