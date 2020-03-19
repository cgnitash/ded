
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

  auto&
      getIO()
  {
    return io_;
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

  void input(std::string name, std::string value);
  void output(std::string name, std::string value);

  void encoding(std::string name, EncodingSpec &e);

  void nestedSubstrate(std::string                   name,
                     SubstrateSpec                 &sub,
                     std::vector<SignalConstraint> input_constraints,
                     std::vector<SignalConstraint> output_constraints);

  void nestedSubstrateVector(std::string name, std::vector<SubstrateSpec> &subs);

  void parseNested(language::Block);
  void parseNestedVector(language::Block);

  std::vector<std::string> serialise(long) const;
  SubstrateSpec            deserialise(std::vector<std::string>);
  std::string              prettyPrint();

  void instantiateUserParameterSizes();

  void setConfigured(bool conf)
  {
	  isConfigurable = conf;
  }

  friend class concepts::Substrate;
private:
	bool isConfigurable = true;

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

  void bindSubstrate(std::string                   name,
                     SubstrateSpec                 sub,
                     std::vector<SignalConstraint> input_constraints,
                     std::vector<SignalConstraint> output_constraints);
  void configureSubstrate(std::string name, SubstrateSpec &sub);
  void bindNestedSubstrateVector(std::string name, std::vector<SubstrateSpec> subs);
  void configureNestedSubstrateVector(std::string                 name,
                                 std::vector<SubstrateSpec> &subs);

  void checkInvalidTokens(language::Block block);
};

}   // namespace specs
}   // namespace ded
