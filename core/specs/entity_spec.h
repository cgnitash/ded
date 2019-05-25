
#pragma once

#include "../configuration.h"
#include "configuration_primitive.h"
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

namespace life {
class entity_spec {
  std::string                                    name_;
  std::map<std::string, configuration_primitive> parameters_;
  std::map<std::string, std::string>             inputs_;
  std::map<std::string, std::string>             outputs_;

public:
  template <typename T> void configure_parameter(std::string name, T &value)
  {
    parameters_[name].get_value(value);
  }

  template <typename T> void bind_parameter(std::string name, T value)
  {
    parameters_[name].set_value(value);
  }

  void bind_input(std::string name, std::string value)
  {
    inputs_[name] = value;
  }

  void bind_output(std::string name, std::string value)
  {
    outputs_[name] = value;
  }

  void configure_input(std::string name, std::string &value)
  {
    value = inputs_[name];
  }

  void configure_output(std::string name, std::string &value)
  {
    value = outputs_[name];
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
      outputs_[key] = std::string{ value };
  }

  configuration to_json() const
  {
    configuration con;
    con["parameters"];
    for (const auto &[key, value] : parameters_)
      con["parameters"][key] = configuration::parse(value.value_as_string());
    for (const auto &[key, value] : inputs_) con["input-tags"][key] = value;
    for (const auto &[key, value] : outputs_) con["output-tags"][key] = value;
    return con;
  }

  void validate_and_merge(entity_spec e)
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
