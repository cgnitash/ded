
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../term_colours.h"
#include "configuration_primitive.h"

namespace life {
class entity_spec {
  struct nested_spec
  {
    std::unique_ptr<entity_spec> e;
    // environment_spec *e = nullptr;
    //std::vector<std::string> pre_constraints;
    //std::vector<std::string> post_constraints;
    nested_spec() = default;
    nested_spec(const nested_spec &ns)
        : e(std::make_unique<entity_spec>(*ns.e))
    {
    }
  };

  std::string                                    name_;
  std::map<std::string, configuration_primitive> parameters_;
  std::map<std::string, std::string>             inputs_;
  std::map<std::string, std::string>             outputs_;
  std::map<std::string, nested_spec>             nested_;

public:
  // entity_spec() = default;

  entity_spec(std::string name = "") : name_(name) {}

  auto name() const { return name_; }

  auto parameters() const { return parameters_; }

  auto inputs() const { return inputs_; }

  auto outputs() const { return outputs_; }

  auto nested() const { return nested_; }

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

  void configure_input(std::string name, std::string &value)
  {
    value = inputs_[name];
  }

  void bind_output(std::string name, std::string value)
  {
    outputs_[name] = value;
  }

  void configure_output(std::string name, std::string &value)
  {
    value = outputs_[name];
  }

  void bind_entity(std::string name, entity_spec env)
  {
    nested_[name].e = std::make_unique<entity_spec>(env);
  }

  void configure_entity(std::string name, entity_spec &e)
  {
    if (!nested_[name].e)
    {
      std::cout << "Warning: <" << name_ << ":" << name
                << "> environment spec has not been bind-ed (probably error)\n";
      //      std::exit(1);
    } else
      e = *nested_[name].e;
  }

  friend std::ostream &operator<<(std::ostream &out, entity_spec e)
  {
    out << term_colours::red_fg << "entity::" << e.name_ << term_colours::reset
        << "\n";

    out << term_colours::yellow_fg << "parameters----" << term_colours::reset
        << "\n";
    for (auto [parameter, value] : e.parameters_)
      out << std::setw(26) << parameter << " : " << value.value_as_string()
          << "\n";
    out << term_colours::yellow_fg << "inputs----" << term_colours::reset
        << "\n";
    for (auto [input, value] : e.inputs_)
      out << std::setw(26) << input << " : " << value << "\n";
    out << term_colours::yellow_fg << "outputs----" << term_colours::reset
        << "\n";
    for (auto [output, value] : e.outputs_)
      out << std::setw(26) << output << " : " << value << "\n";
    return out;
  }
};

/*
std::ostream &
    operator<<(std::ostream &out, entity_spec e)
{

  out << term_colours::red_fg << "entity::" << e.name() << term_colours::reset
      << "\n";

  out << term_colours::yellow_fg << "parameters----" << term_colours::reset
      << "\n";
  for (auto [parameter, value] : e.parameters())
    out << std::setw(26) << parameter << " : " << value.value_as_string()
        << "\n";
  out << term_colours::yellow_fg << "inputs----" << term_colours::reset << "\n";
  for (auto [input, value] : e.inputs())
    out << std::setw(26) << input << " : " << value << "\n";
  out << term_colours::yellow_fg << "outputs----" << term_colours::reset
      << "\n";
  for (auto [output, value] : e.outputs())
    out << std::setw(26) << output << " : " << value << "\n";
  return out;
}
*/
}   // namespace life
