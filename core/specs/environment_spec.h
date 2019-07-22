
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <range/v3/all.hpp>
#include <regex>
#include <string>

#include "../language/parser.h"
#include "../utilities/term_colours.h"
#include "configuration_primitive.h"
#include "population_spec.h"
#include "signal_spec.h"

namespace ded
{

// forward declaration to provide friendship
namespace concepts
{
class Environment;
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

class EnvironmentSpec
{

  struct NestedSpec
  {
    std::unique_ptr<EnvironmentSpec> e;
    struct
    {
      std::vector<std::string> pre_;
      std::vector<std::string> post_;
    } constraints_;
    NestedSpec() = default;
    NestedSpec(const NestedSpec &ns)
        : e(std::make_unique<EnvironmentSpec>(*ns.e)),
          constraints_(ns.constraints_)
    {
    }
  };

  TraceConfig traces_;

  std::string                                   name_;
  std::string                                   user_specified_name_;
  std::map<std::string, ConfigurationPrimitive> parameters_;

  IO io_;

  Tags tags_;

  std::map<std::string, NestedSpec> nested_;

  std::vector<std::pair<std::pair<std::string, std::string>,
                        std::pair<std::string, std::string>>>
      tag_flow_equalities_;
  std::vector<std::pair<std::pair<std::string, std::string>,
                        std::pair<std::string, std::string>>>
      tag_flow_inequalities_;

  // Parser parser_;
  // Block block_;

public:
  // EnvironmentSpec() = default;
  //~EnvironmentSpec() = default;

  EnvironmentSpec(std::string name = "") : name_(name)
  {
  }
  // EnvironmentSpec(Parser);
  EnvironmentSpec(language::Parser, language::Block);

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
      get_user_specified_name() const
  {
    return user_specified_name_;
  }
  auto
      set_user_specified_name(std::string name)
  {
    user_specified_name_ = name;
  }

  template <typename T>
  void
      bind_parameter(std::string name, T value)
  {
    parameters_[name].set_value(value);
  }

  template <typename T>
  void
      configure_parameter(std::string name, T &value)
  {
    parameters_[name].get_value(value);
  }

  void
      bind_pre(std::string name, std::string value)
  {
    tags_.pre_.push_back({ name, SignalSpec{ name, name, value } });
  }

  void
      configure_pre(std::string name, std::string &value)
  {
    value =
        ranges::find_if(tags_.pre_, [=](auto ns) { return ns.first == name; })
            ->second.identifier();
  }

  void
      bind_post(std::string name, std::string value)
  {
    tags_.post_.push_back({ name, SignalSpec{ name, name, value } });
  }

  void
      configure_post(std::string name, std::string &value)
  {
    value =
        ranges::find_if(tags_.post_, [=](auto ns) { return ns.first == name; })
            ->second.identifier();
  }

  void
      bind_input(std::string name, std::string value)
  {
    io_.inputs_.push_back({ name, SignalSpec{ name, name, value } });
  }

  void
      configure_input(std::string name, std::string &value)
  {
    value =
        ranges::find_if(io_.inputs_, [=](auto ns) { return ns.first == name; })
            ->second.identifier();
  }

  void
      bind_output(std::string name, std::string value)
  {
    io_.outputs_.push_back({ name, SignalSpec{ name, name, value } });
  }

  void
      configure_output(std::string name, std::string &value)
  {
    value =
        ranges::find_if(io_.outputs_, [=](auto ns) { return ns.first == name; })
            ->second.identifier();
  }

  void
      bind_environment(std::string name, EnvironmentSpec env)
  {
    nested_[name].e = std::make_unique<EnvironmentSpec>(env);
  }

  void
      configure_environment(std::string name, EnvironmentSpec &e)
  {
    e = *nested_[name].e;
  }

  void
      bind_environment_pre_constraints(std::string              name,
                                       std::vector<std::string> pre_constraints)
  {
    nested_[name].constraints_.pre_ = pre_constraints;
  }

  void
      bind_environment_post_constraints(
          std::string              name,
          std::vector<std::string> post_constraints)
  {
    nested_[name].constraints_.post_ = post_constraints;
  }

  void
      bind_tag_equality(std::pair<std::string, std::string> x,
                        std::pair<std::string, std::string> y)
  {
    tag_flow_equalities_.push_back({ x, y });
  }

  void
      bind_tag_inequality(std::pair<std::string, std::string> x,
                          std::pair<std::string, std::string> y)
  {
    tag_flow_inequalities_.push_back({ x, y });
  }

  // friend std::ostream &operator<<(std::ostream &out, const EnvironmentSpec
  // &e)
  std::vector<std::string> dump(long depth, bool) const;
  EnvironmentSpec          parse(std::vector<std::string> pop_dump);

  std::string pretty_print();

  // void parser_parse();
  void instantiate_user_parameter_sizes();
  void bind_entity_io(IO);
  void bind_tags(int);
  std::vector<std::pair<Trace, std::string>> record_traces();

  friend class concepts::Environment;
};

}   // namespace specs
}   // namespace ded
