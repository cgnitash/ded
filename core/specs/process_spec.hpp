
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <string>

#include "../language/parser.hpp"
#include "../utilities/term_colours.hpp"
#include "../utilities/utilities.hpp"
#include "configuration_primitive.hpp"
#include "converter_spec.hpp"
#include "population_spec.hpp"
#include "signal_spec.hpp"

namespace ded
{

// forward declaration to provide friendship
namespace concepts
{
class Process;
}

namespace specs
{

struct Trace
{
  std::string name_;
  int         frequency_;
};

struct TraceConfig
{
  std::vector<Trace> pre_;
  std::vector<Trace> post_;
};

class ProcessSpec
{
public:
  ProcessSpec(std::string name = "") : name_(name)
  {
  }

  ProcessSpec(language::Block);

  auto
      name() const
  {
    return name_;
  }

  auto
      traces() const
  {
    return traces_;
  }

  auto
      getTagConversions()
  {
    return tag_conversions_;
  }

  auto
      getUserSpecifiedName() const
  {
    return user_specified_name_;
  }

  auto
      setUserSpecifiedName(std::string name)
  {
    user_specified_name_ = name;
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

  void preTag(std::string name, std::string value);

  void postTag(std::string name, std::string value);

  void input(std::string                  name,
             std::string                  value,
             ConversionSignatureSequence &input);

  void output(std::string                  name,
              std::string                  value,
              ConversionSignatureSequence &output);

  void nestedProcess(std::string                   name,
                     ProcessSpec &                 proc,
                     std::vector<SignalConstraint> pre_constraints  = {},
                     std::vector<SignalConstraint> post_constraints = {});

  void nestedProcessVector(std::string                   name,
                           std::vector<ProcessSpec> &    procs,
                           std::vector<SignalConstraint> pre_constraints  = {},
                           std::vector<SignalConstraint> post_constraints = {});

  std::vector<std::string> serialise(long, bool) const;
  ProcessSpec              deserialise(std::vector<std::string>);
  std::string              prettyPrint();

  void                                       instantiateUserParameterSizes();
  void                                       bindSubstrateIO(SubstrateSpec);
  std::vector<std::pair<Trace, std::string>> queryTraces();

  void bindTags();

  void
      setConfigured(bool conf)
  {
    isConfigurable = conf;
  }

  friend class concepts::Process;

private:
  bool isConfigurable = true;

  struct NestedProcessSpec
  {
    std::unique_ptr<ProcessSpec> e;
    Tags                         constraints_;
    NestedProcessSpec() = default;
    NestedProcessSpec(const NestedProcessSpec &ns)
        : e(std::make_unique<ProcessSpec>(*ns.e)), constraints_(ns.constraints_)
    {
    }
  };

  TraceConfig traces_;

  std::string     name_;
  language::Token name_token_;
  std::string     user_specified_name_;
  Parameters      parameters_;

  // might not be required
  IO io_;
  // since replaced by ??
  std::vector<language::Block::TokenBlockSignalBind> input_conversion_sequence_;
  std::vector<language::Block::TokenBlockSignalBind>
      output_conversion_sequence_;
  std::vector<language::Block::TokenBlockSignalBind> tag_conversion_sequence_;
  ConversionSequence                                 input_conversions_;
  ConversionSequence                                 output_conversions_;

  Tags               tags_;
  ConversionSequence tag_conversions_;

  std::map<std::string, NestedProcessSpec> nested_;
  std::map<std::string, std::pair<std::vector<NestedProcessSpec>, Tags>>
      nested_vector_;

  void bindInput(std::string name, std::string value);

  void configureInput(std::string name, ConversionSignatureSequence &input);

  void bindOutput(std::string name, std::string value);

  void configureOutput(std::string name, ConversionSignatureSequence &output);

  void bindProcess(std::string name, ProcessSpec proc);

  void configureProcess(std::string name, ProcessSpec &proc);

  void bindProcessVector(std::string name, std::vector<ProcessSpec> procs);

  void configureProcessVector(std::string               name,
                              std::vector<ProcessSpec> &procs);

  void bindProcessPreConstraints(std::string                   proc_name,
                                 std::vector<SignalConstraint> pre_constraints);

  void bindProcessPostConstraints(
      std::string                   proc_name,
      std::vector<SignalConstraint> post_constraints);

  void bindProcessVectorPreConstraints(
      std::string                   proc_name,
      std::vector<SignalConstraint> pre_constraints);

  void bindProcessVectorPostConstraints(
      std::string                   proc_name,
      std::vector<SignalConstraint> post_constraints);

  void parseSignalBinds(language::Block);
  void parseTagBinds(language::Block block);
  void parseTraces(language::Block);
  void parseNested(language::Block);
  void parseNestedVector(language::Block);

  std::pair<NamedSignal, std::string> getTagsWithName(language::Token token,
                                                      bool            is_pre);

  void bindTagConversionSequence(
      language::Block::TokenBlockSignalBind signal_conversion_sequence);

  void bindSignalConversionSequence(
      language::Block::TokenBlockSignalBind signal_conversion_sequence,
      specs::SubstrateSpec &                sub_spec,
      bool                                  is_input);

  void convertSignalConversionSequence(language::Block::TokenBlocks converter,
                                       SignalSpec &                 sig,
                                       language::Token &    source_token,
                                       ConversionSequence_ &cs);

  void checkInvalidTokens(language::Block block);

  std::string prettyPrintNested();
  std::string prettyPrintNestedVector();
};

}   // namespace specs
}   // namespace ded
