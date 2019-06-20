
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <range/v3/all.hpp>
#include <regex>
#include <string>

#include "../parser/parser.h"
#include "../term_colours.h"
#include "configuration_primitive.h"
#include "population_spec.h"
#include "signal_spec.h"

namespace life {

struct trace
{
  signal_spec signal_;
  int         frequency_;
};

struct trace_config
{
  std::vector<trace> pre_;
  std::vector<trace> post_;
};

class environment_spec {
  struct nested_spec
  {
    std::unique_ptr<environment_spec> e;
    struct
    {
      std::vector<std::string> pre_;
      std::vector<std::string> post_;
    } constraints_;
    nested_spec() = default;
    nested_spec(const nested_spec &ns)
        : e(std::make_unique<environment_spec>(*ns.e)),
          constraints_(ns.constraints_)
    {
    }
  };

  trace_config traces_;

  std::string                                    name_;
  std::string                                    user_specified_name_;
  std::map<std::string, configuration_primitive> parameters_;
  struct
  {
    std::map<std::string, signal_spec> inputs_;
    std::map<std::string, signal_spec> outputs_;
  } io_;
  struct
  {
    std::map<std::string, signal_spec> pre_;
    std::map<std::string, signal_spec> post_;
  } tags_;
  std::map<std::string, nested_spec> nested_;
  std::vector<std::pair<std::pair<std::string, std::string>,
                        std::pair<std::string, std::string>>>
      tag_flow_equalities;
  std::vector<std::pair<std::pair<std::string, std::string>,
                        std::pair<std::string, std::string>>>
      tag_flow_inequalities;

public:
  // environment_spec() = default;
  //~environment_spec() = default;

  environment_spec(std::string name = "") : name_(name) {}
  // environment_spec(parser);
  environment_spec(parser, block);

  auto name() const { return name_; }

  auto traces() const { return traces_; }

  auto get_user_specified_name() const { return user_specified_name_; }
  auto set_user_specified_name(std::string name)
  {
    user_specified_name_ = name;
  }
  /*

  auto parameters() const { return parameters_; }

  auto inputs() const { return io_.inputs_; }

  auto outputs() const { return io_.outputs_; }

  auto pre_tags() const { return tags_.pre_; }

  auto post_tags() const { return tags_.post_; }

  auto pre_traces() const { return traces_.pre_; }

  auto post_traces() const { return traces_.post_; }


  auto nested() const { return nested_; }

  auto tag_flow_constraints() const { return tag_flow_constraints_; }
  */

  template <typename T> void bind_parameter(std::string name, T value)
  {
    parameters_[name].set_value(value);
  }

  template <typename T> void configure_parameter(std::string name, T &value)
  {
    parameters_[name].get_value(value);
  }

  void bind_pre(std::string name, std::string value)
  {
    tags_.pre_[name] = signal_spec{ name, name, value };
  }

  void configure_pre(std::string name, std::string &value)
  {
    value = tags_.pre_[name].id_type_specifier() ;
  }

  void bind_post(std::string name, std::string value)
  {
    tags_.post_[name] =  signal_spec{ name, name, value };
  }

  void configure_post(std::string name, std::string &value)
  {
    value = tags_.post_[name].id_type_specifier();
  }

  void bind_input(std::string name, std::string value)
  {
    io_.inputs_[name] = signal_spec{ name, name, value };
  }

  void configure_input(std::string name, std::string &value)
  {
    value = io_.inputs_[name].id_type_specifier();
  }

  void bind_output(std::string name, std::string value)
  {
    io_.outputs_[name] = signal_spec{ name, name, value };
  }

  void configure_output(std::string name, std::string &value)
  {
    value = io_.outputs_[name].id_type_specifier();
  }

  void bind_environment(std::string name, environment_spec env)
  {
    nested_[name].e = std::make_unique<environment_spec>(env);
  }

  void configure_environment(std::string name, environment_spec &e)
  {
    e = *nested_[name].e;
  }

  void
      bind_environment_pre_constraints(std::string              name,
                                       std::vector<std::string> pre_constraints)
  {
    nested_[name].constraints_.pre_ = pre_constraints;
  }

  void bind_environment_post_constraints(
      std::string name, std::vector<std::string> post_constraints)
  {
    nested_[name].constraints_.post_ = post_constraints;
  }

  void bind_tag_equality(std::pair<std::string, std::string> x,
                         std::pair<std::string, std::string> y)
  {
    tag_flow_equalities.push_back({ x, y });
  }

  void bind_tag_inequality(std::pair<std::string, std::string> x,
                           std::pair<std::string, std::string> y)
  {
    tag_flow_inequalities.push_back({ x, y });
  }

  // friend std::ostream &operator<<(std::ostream &out, const environment_spec
  // &e)
  std::string      dump(long depth);
  environment_spec parse(std::vector<std::string> pop_dump);

  std::string pretty_print();

  /*
  void bind_all_entity_signals(population_spec);
  void bind_all_entity_inputs(population_spec);
  void bind_all_entity_outputs(population_spec);
  */
};

}   // namespace life
