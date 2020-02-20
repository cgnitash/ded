
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
//  std::map<std::string, ConfigurationPrimitive> parameters_;
  Parameters                                              parameters_;
  std::pair<SignalSpec,SignalSpec> args_; 

public:

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

  auto
      args() const
  {
    return args_;
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

  void bindFrom(std::string from)
  {
	args_.first = SignalSpec{"","",from};
  }

  void bindTo(std::string to)
  {
	args_.second = SignalSpec{"","",to};
  }

   std::vector<std::string>  serialise(long )const;
  ConverterSpec deserialise(std::vector<std::string> );
  std::string    prettyPrint();

  //  friend std::ostream &operator<<(std::ostream &out, ConverterSpec e)

  friend class concepts::Converter;
};

/*
struct ConverterSink
{
std::vector<ConverterSpec> sequence_;
std::string sink_;
};

ConverterSink parseConversionSequence(language::Token);
*/
}   // namespace specs
}   // namespace ded
