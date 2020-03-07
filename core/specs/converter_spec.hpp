
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

class ConverterSpec
{
  std::string                       name_;
  Parameters                        parameters_;
  std::pair<SignalSpec, SignalSpec> args_;

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

  friend class concepts::Converter;
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
    sliceConverter(long from, long every)
{

  return [=](concepts::Signal s) -> concepts::Signal {
    auto v = std::any_cast<std::vector<T>>(s);
    return v | rv::drop_exactly(from) | rv::stride(every) |
           rs::to<std::vector<T>>;
  };
}

ConversionSignature makeSliceConverter(long from, long every, std::string vtt);
}   // namespace specs
}   // namespace ded
