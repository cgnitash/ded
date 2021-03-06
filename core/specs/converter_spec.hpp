
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../concepts/signal.hpp"
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

struct SliceRange
{
  long        from = 0, to = 0, every = 0;
  std::string vtt{};
};

class ConverterSpec
{
public:
  ConverterSpec(std::string name = "") : name_(name)
  {
  }

  ConverterSpec(language::Block);

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
      parameter(std::string                           name,
                    ArgumentType                          &value,
                    std::vector<Constraint<ArgumentType>> cons = {})
  {
    if (isConfigurable)
      parameters_.configure(name, value);
    else
      parameters_.bind(name, value, cons);
  }

  template <typename ArgumentType>
  void
      forceParameter(std::string name, ArgumentType value)
  {
    parameters_.parameters_[name].setValue(value);
  }

  SliceRange
      getSliceRange()
  {
	  long from = 0, to = 0, every = 0;
	  std::string vtt;
	  parameters_.configure("from", from);
	  parameters_.configure("to", to);
	  parameters_.configure("every", every);
	  parameters_.configure("vtt", vtt);
	  return {from, to , every, vtt};
  }

  void
      bindFrom(std::string from)
  {
    args_.first = SignalSpec{ from };
  }

  void
      bindTo(std::string to)
  {
    args_.second = SignalSpec{ to };
  }

  std::vector<std::string> serialise(long) const;
  ConverterSpec            deserialise(std::vector<std::string>);
  std::string              prettyPrint();

  //  friend std::ostream &operator<<(std::ostream &out, ConverterSpec e)
  void setConfigured(bool conf)
  {
	  isConfigurable = conf;
  }


  friend class concepts::Converter;

private:
  bool isConfigurable = true;

  std::string                       name_;
  Parameters                        parameters_;
  std::pair<SignalSpec, SignalSpec> args_;

  void checkInvalidTokens(language::Block block);
};

// private

struct ConversionSequence_
{
  std::string                source_;
  std::string                sink_;
  std::vector<ConverterSpec> specs_;
};

using ConversionSequence = std::vector<ConversionSequence_>;

using ConversionSignature = std::function<concepts::Signal(concepts::Signal)>;

struct ConversionSignatureSequence_
{
  std::string                      source_;
  std::string                      sink_;
  std::vector<ConversionSignature> sequence_;
};

using ConversionSignatureSequence = std::vector<ConversionSignatureSequence_>;

template <typename T>
ConversionSignature
    sliceConverter(long from, long to, long every)
{

  return [=](concepts::Signal s) -> concepts::Signal {
    auto v = std::any_cast<std::vector<T>>(s);
    return v | rv::slice(from, to) | rv::stride(every) |
           rs::to<std::vector<T>>;
  };
}

ConversionSignature
    makeSliceConverter(long from, long to, long every, std::string vtt);

ConversionSignatureSequence_ makeConversionSignatureSequence(ConversionSequence_);	
}   // namespace specs
}   // namespace ded
