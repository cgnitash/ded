
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../language/parser.h"
#include "../language/token.h"
#include "../utilities/term_colours.h"
#include "../utilities/utilities.h"
#include "configuration_primitive.h"
#include "encoding_spec.h"
#include "signal_spec.h"

namespace ded
{

// forward declaration to provide friendship
namespace concepts
{
class Substrate;
}

namespace specs
{

class SubstrateSpec
{
  struct NestedSubstrateSpec
  {
    std::unique_ptr<SubstrateSpec> e;
    IO                             constraints_;
    NestedSubstrateSpec() = default;
    NestedSubstrateSpec(const NestedSubstrateSpec &ns)
        : e(std::make_unique<SubstrateSpec>(*ns.e)),
          constraints_(ns.constraints_)
    {
    }
  };

  std::string                                             name_;
  language::Token                                         name_token_;
  std::map<std::string, ConfigurationPrimitive>           parameters_;
  std::map<std::string, NestedSubstrateSpec>              nested_;
  std::map<std::string, std::vector<NestedSubstrateSpec>> nested_vector_;
  std::map<std::string, EncodingSpec>                     encodings_;
  IO                                                      io_;

public:
  SubstrateSpec(std::string name = "") : name_(name)
  {
  }
  SubstrateSpec(language::Block);

  auto
      name() const
  {
    return name_;
  }

  auto
      nameToken() const
  {
    return name_token_;
  }

  auto
      getIO()
  {
    return io_;
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

  void bindInput(std::string name, std::string value);

  void configureInput(std::string name, std::string &value);
  void bindOutput(std::string name, std::string value);
  void configureOutput(std::string name, std::string &value);
  void bindEncoding(std::string name, EncodingSpec e);
  void configureEncoding(std::string name, EncodingSpec &e);
  void bindNestedSubstrates(std::string name, std::vector<SubstrateSpec> subs);
  void configureNestedSubstrates(std::string                 name,
                                 std::vector<SubstrateSpec> &subs);
  void bindSubstrate(
      std::string                                      name,
      SubstrateSpec                                    sub,
      std::vector<std::pair<std::string, std::string>> input_constraints,
      std::vector<std::pair<std::string, std::string>> output_constraints);
  void configureSubstrate(std::string name, SubstrateSpec &sub);
  void parseParameters( language::Block);
  void parseNested( language::Block);
  void parseNestedVector( language::Block);

  std::vector<std::string> serialise(long) const;
  SubstrateSpec            deserialise(std::vector<std::string>);
  std::string              prettyPrint();

  void instantiateUserParameterSizes(int);
  void checkNestedIO();
  void updateNestedConstraints(SignalSpecSet &constraints);
  void matchNestedSignals(NestedSubstrateSpec &,
                          SignalSpecSet &source_tags,
                          SignalSpecSet &sink_tags,
                          bool           is_input);
  void errSignalBind(SubstrateSpec sub_spec, SignalSpec sig, bool is_input);

  void bindSubstrateIO(SubstrateSpec);

  friend class concepts::Substrate;
};

}   // namespace specs
}   // namespace ded
