
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <range/v3/all.hpp>
#include <regex>
#include <string>

#include "../term_colours.h"
#include "configuration_primitive.h"
#include "trace.h"

namespace life {
class environment_spec {

  struct nested_spec
  {
    std::unique_ptr<environment_spec> e;
    // environment_spec *e = nullptr;
    struct
    {
      std::vector<std::string> pre_;
      std::vector<std::string> post_;
    } constraints_;
    nested_spec() = default;
    nested_spec(const nested_spec &ns)
        : e(std::make_unique<environment_spec>(*ns.e)),
          constraints_(ns.constraints_)
    // constraints_.post_(ns.constraints_.post_)
    {
    }
    //~nested_spec() { delete e; }
  };

  /*
  struct trace_config 
  {
    std::vector<trace> pre_;
    std::vector<trace> post_;
  } ; */
 trace_config traces_;

  std::string                                    name_;
  std::map<std::string, configuration_primitive> parameters_;
  struct
  {
    std::map<std::string, std::string> inputs_;
    std::map<std::string, std::string> outputs_;
  } io_;
  struct
  {
    std::map<std::string, std::string> pre_;
    std::map<std::string, std::string> post_;
  } tags_;
  std::map<std::string, nested_spec> nested_;
  std::vector<std::pair<std::pair<std::string, std::string>,
                        std::pair<std::string, std::string>>>
      tag_flow_constraints_;

public:
  // environment_spec() = default;
  //~environment_spec() = default;

  environment_spec(std::string name = "") : name_(name) {}

  auto name() const { return name_; }

  auto parameters() const { return parameters_; }

  auto inputs() const { return io_.inputs_; }

  auto outputs() const { return io_.outputs_; }

  auto pre_tags() const { return tags_.pre_; }

  auto post_tags() const { return tags_.post_; }

  auto pre_traces() const { return traces_.pre_; }

  auto post_traces() const { return traces_.post_; }

  auto traces() const { return traces_; }

  auto nested() const { return nested_; }

  auto tag_flow_constraints() const { return tag_flow_constraints_; }

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
    tags_.pre_[name] = value;
  }

  void configure_pre(std::string name, std::string &value)
  {
    value = tags_.pre_[name];
  }

  void bind_post(std::string name, std::string value)
  {
    tags_.post_[name] = value;
  }

  void configure_post(std::string name, std::string &value)
  {
    value = tags_.post_[name];
  }

  void bind_input(std::string name, std::string value)
  {
    io_.inputs_[name] = value;
  }

  void configure_input(std::string name, std::string &value)
  {
    value = io_.inputs_[name];
  }

  void bind_output(std::string name, std::string value)
  {
    io_.outputs_[name] = value;
  }

  void configure_output(std::string name, std::string &value)
  {
    value = io_.outputs_[name];
  }

  void bind_environment(std::string name, environment_spec env)
  {
    nested_[name].e = std::make_unique<environment_spec>(env);
  }

  void configure_environment(std::string name, environment_spec &e)
  {
    /*
if (!nested_[name].e)
{
std::cout << "Warning: <" << name_ << ":" << name
          << "> environment spec has not been bind-ed (probably error)\n";
//      std::exit(1);
} else
  */
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

  void bind_tag_flow(std::pair<std::string, std::string> x,
                     std::pair<std::string, std::string> y)
  {
    tag_flow_constraints_.push_back({ x, y });
  }

  // friend std::ostream &operator<<(std::ostream &out, const environment_spec
  // &e)
  std::string      dump(long depth);
  environment_spec parse(std::vector<std::string> pop_dump);

  std::string pretty_print();
};

}   // namespace life
