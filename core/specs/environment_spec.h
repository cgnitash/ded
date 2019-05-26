
#pragma once

#include "../configuration.h"
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
    std::vector<std::string>          pre_constraints;
    std::vector<std::string>          post_constraints;
  };

  std::string                                    name_;
  std::map<std::string, configuration_primitive> parameters_;
  std::map<std::string, std::string>             inputs_;
  std::map<std::string, std::string>             outputs_;
  std::map<std::string, std::string>             pre_tags_;
  std::map<std::string, std::string>             post_tags_;
  std::map<std::string, nested_spec>             nested_;

public:
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

  void bind_environment(std::string              name,
                        environment_spec         env,
                        std::vector<std::string> pre_constraints,
                        std::vector<std::string> post_constraints)
  {
    nested_[name] =
        nested_spec{ std::make_unique<environment_spec>(env), pre_constraints, post_constraints };
  }

  void configure_environment(std::string name, environment_spec &e)
  {
    e = *nested_[name].e;
  }

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
};

}   // namespace life
