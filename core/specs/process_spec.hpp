
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
  SignalSpec signal_;
  int        frequency_;
};

struct TraceConfig
{
  std::vector<Trace> pre_;
  std::vector<Trace> post_;
};

class ProcessSpec
{

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

  std::string                                   name_;
  language::Token                           name_token_;
  std::string                                   user_specified_name_;
  std::map<std::string, ConfigurationPrimitive> parameters_;

  IO io_;

  Tags tags_;

  std::vector<std::pair<language::Token, language::Token>> signal_binds_;

  std::map<std::string, NestedProcessSpec>              nested_;
  std::map<std::string, std::pair<std::vector<NestedProcessSpec>, Tags>>
      nested_vector_;

  std::vector<std::pair<std::pair<std::string, std::string>,
                        std::pair<std::string, std::string>>>
      tag_flow_equalities_;
  std::vector<std::pair<std::pair<std::string, std::string>,
                        std::pair<std::string, std::string>>>
      tag_flow_inequalities_;

  void parseParameters( language::Block);
  void parseSignalBinds( language::Block);
  void parseTraces( language::Block);
  void parseNested( language::Block);
  void parseNestedVector( language::Block);

  void matchTags(SignalSpecSet &source_tags,
                 SignalSpecSet &sink_tags,
                 int &          tag_count);
  void updateAndMatchTags(SignalSpecSet &source_tags,
                          SignalSpecSet &sink_tags,
                          int &          tag_count);
  bool attemptExplicitBind(SignalSpec &    sig,
                           SignalSpecSet   sub_sigs,
                           language::Token sub_spec_name,
                           bool            is_input);
  void errSignalBind(SignalSpec      proc_sig,
                     language::Token sub_spec_name,
                     SignalSpecSet   sub_sigs,
                     bool);
  void bindSignalsTo(SignalSpecSet sigs,
                     language::Token   sub_spec_name,
                     bool          is_input);
  void updateNestedConstraints(SignalSpecSet &constraints);
  void matchTagFlowEqualities(int &tag_count);
  void matchNestedTagConstraints(int &tag_count);
  std::string prettyPrintNested();
  std::string prettyPrintNestedVector();

public:

  ProcessSpec(std::string name = "") : name_(name)
  {
  }

  ProcessSpec( language::Block);

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
      getUserSpecifiedName() const
  {
    return user_specified_name_;
  }

  auto
      setUserSpecifiedName(std::string name)
  {
    user_specified_name_ = name;
  }

  template <typename ValueType>
  void
      bindParameter(
          std::string name,
          ValueType   value,
          std::vector<std::pair<std::function<bool(ValueType)>, std::string>>
              cons = {})

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

  template <typename ValueType>
  void
      configureParameter(std::string name, ValueType &value)
  {
    if (parameters_.find(name) == parameters_.end())
    {
      std::cout << "User error: parameter " << name
                << " has not been declared\n";
      throw SpecError{};
    }
    parameters_[name].get_value(value);
  }

  void
      bindPreTag(std::string name, std::string value);

  void
      configurePreTag(std::string name, std::string &value);

  void
      bindPostTag(std::string name, std::string value);

  void
      configurePostTag(std::string name, std::string &value);

  void
      bindInput(std::string name, std::string value);

  void
      configureInput(std::string name, std::string &value);

  void
      bindOutput(std::string name, std::string value);

  void
      configureOutput(std::string name, std::string &value);

  void bindProcess(std::string name, ProcessSpec proc);

  void configureProcess(std::string name, ProcessSpec &proc);

  void bindProcessVector(std::string name, std::vector<ProcessSpec> procs);

  void configureProcessVector(std::string               name,
                              std::vector<ProcessSpec> &procs);

  void bindProcessPreConstraints(
      std::string                                      proc_name,
      std::vector<std::pair<std::string, std::string>> pre_constraints);

  void bindProcessPostConstraints(
      std::string                                      proc_name,
      std::vector<std::pair<std::string, std::string>> post_constraints);

  void bindProcessVectorPreConstraints(
      std::string                                      proc_name,
      std::vector<std::pair<std::string, std::string>> pre_constraints);

  void bindProcessVectorPostConstraints(
      std::string                                      proc_name,
      std::vector<std::pair<std::string, std::string>> post_constraints);

  void bindTagEquality(std::pair<std::string, std::string> x,
                       std::pair<std::string, std::string> y);

  std::vector<std::string> serialise(long, bool) const;
  ProcessSpec              deserialise(std::vector<std::string>);
  std::string prettyPrint();

  void                                       instantiateUserParameterSizes();
  void                                       bindSubstrateIO(SubstrateSpec);
  void                                       bindTags(int);
  void                                       recordTraces();
  std::vector<std::pair<Trace, std::string>> queryTraces();

  friend class concepts::Process;
};

}   // namespace specs
}   // namespace ded
