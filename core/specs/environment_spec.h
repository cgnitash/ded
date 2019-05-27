
#pragma once

//#include "../configuration.h"
#include "configuration_primitive.h"
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <string>
#include <variant>

namespace life {
class environment_spec {

  struct nested_spec
  {
    std::unique_ptr<environment_spec> e;
    // environment_spec *e = nullptr;
    std::vector<std::string> pre_constraints;
    std::vector<std::string> post_constraints;
    nested_spec() = default;
    nested_spec(const nested_spec &ns)
        : e(std::make_unique<environment_spec>(*ns.e)),
          pre_constraints(ns.pre_constraints),
          post_constraints(ns.post_constraints)
    {
    }
    //~nested_spec() { delete e; }
  };

  std::string                                    name_;
  std::map<std::string, configuration_primitive> parameters_;
  std::map<std::string, std::string>             inputs_;
  std::map<std::string, std::string>             outputs_;
  std::map<std::string, std::string>             pre_tags_;
  std::map<std::string, std::string>             post_tags_;
  std::map<std::string, nested_spec>             nested_;
  std::vector<std::pair<std::pair<std::string, std::string>,
                        std::pair<std::string, std::string>>>
      tag_flow_constraints_;

public:
  // environment_spec() = default;
  //~environment_spec() = default;

  environment_spec(std::string name = "") : name_(name) {}

  auto name() const { return name_; }

  auto parameters() const { return parameters_; }

  auto inputs() const { return inputs_; }

  auto outputs() const { return outputs_; }

  auto pre_tags() const { return pre_tags_; }

  auto post_tags() const { return post_tags_; }

  auto nested() const { return nested_; }

  auto tag_flow_constraints() const { return tag_flow_constraints_; }

  template <typename T> void configure_parameter(std::string name, T &value)
  {
    parameters_[name].get_value(value);
  }

  template <typename T> void bind_parameter(std::string name, T value)
  {
    parameters_[name].set_value(value);
  }

  void bind_pre(std::string name, std::string value)
  {
    pre_tags_[name] = value;
  }

  void bind_post(std::string name, std::string value)
  {
    post_tags_[name] = value;
  }

  void bind_input(std::string name, std::string value)
  {
    inputs_[name] = value;
  }

  void bind_output(std::string name, std::string value)
  {
    outputs_[name] = value;
  }

  void configure_pre(std::string name, std::string &value)
  {
    value = pre_tags_[name];
  }

  void configure_post(std::string name, std::string &value)
  {
    value = post_tags_[name];
  }

  void configure_input(std::string name, std::string &value)
  {
    value = inputs_[name];
  }

  void configure_output(std::string name, std::string &value)
  {
    value = outputs_[name];
  }

  void bind_environment(std::string name, environment_spec env)
  {
    //if (!env.name().empty())
      // nested_[name].e = new environment_spec{env};
      nested_[name].e = std::make_unique<environment_spec>(env);
  }

  void
      bind_environment_pre_constraints(std::string              name,
                                       std::vector<std::string> pre_constraints)
  {
    nested_[name].pre_constraints = pre_constraints;
  }

  void bind_environment_post_constraints(
      std::string name, std::vector<std::string> post_constraints)
  {
    nested_[name].post_constraints = post_constraints;
  }

  void configure_environment(std::string name, environment_spec &e)
  {
    if (!nested_[name].e)
    {
      std::cout << "Warning: <" << name_ << ":" << name
                << "> environment spec has not been bind-ed (probably error)\n";
      //      std::exit(1);
    } else
      e = *nested_[name].e;
  }

  void bind_tag_flow(std::pair<std::string, std::string> x,
                     std::pair<std::string, std::string> y)
  {
    tag_flow_constraints_.push_back({ x, y });
  }

  /*
  void from_json(configuration con)
  {
    if (con.find("parameters") == con.end()) return;
    const auto &params = con["parameters"];
    for (const auto &[key, value] : params.items())
    {
      std::stringstream ss;
      ss << value;
      parameters_[key].parse(ss.str());
    }
    const auto &inputs = con["input-tags"];
    for (const auto &[key, value] : inputs.items())
      inputs_[key] = std::string{ value };
    const auto &outputs = con["output-tags"];
    for (const auto &[key, value] : outputs.items())
      inputs_[key] = std::string{ value };
    const auto &pre_tags = con["pre-tags"];
    for (const auto &[key, value] : pre_tags.items())
      inputs_[key] = std::string{ value };
    const auto &post_tags = con["post-tags"];
    for (const auto &[key, value] : post_tags.items())
      outputs_[key] = std::string{ value };
  }

  configuration to_json() const
  {
    configuration con;
    con["parameters"];
    for (const auto &[key, value] : parameters_)
      con["parameters"][key] = configuration::parse(value.value_as_string());
    con["input-tags"];
    for (const auto &[key, value] : inputs_) con["input-tags"][key] = value;
    con["output-tags"];
    for (const auto &[key, value] : outputs_) con["output-tags"][key] = value;
    con["pre-tags"];
    for (const auto &[key, value] : pre_tags_) con["pre-tags"][key] = value;
    con["post-tags"];
    for (const auto &[key, value] : post_tags_) con["post-tags"][key] = value;
    return con;
  }

  void validate_and_merge(environment_spec e)
  {
    for (const auto &[key, value] : e.parameters_)
    {
      if (parameters_.find(key) == parameters_.end())
      {
        std::cout << "Error: Configuration mismatch -- \"" << key
                  << "\" is not a valid parameter\n";
        exit(1);
      }
      parameters_[key] = value;
    }
  }
  */
};

}   // namespace life
