
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../language/parser.hpp"
#include "../language/token.hpp"
#include "../utilities/term_colours.hpp"
#include "../utilities/utilities.hpp"
#include "configuration_primitive.hpp"
#include "encoding_spec.hpp"
#include "signal_spec.hpp"

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
  Parameters                                              parameters_;
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
      std::vector<SignalConstraint> input_constraints,
      std::vector<SignalConstraint> output_constraints);
  void configureSubstrate(std::string name, SubstrateSpec &sub);
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
