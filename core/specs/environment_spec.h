
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <string>
#include <variant>

#include "../term_colours.h"
#include "configuration_primitive.h"

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
    pre_tags_[name] = value;
  }

  void configure_pre(std::string name, std::string &value)
  {
    value = pre_tags_[name];
  }

  void bind_post(std::string name, std::string value)
  {
    post_tags_[name] = value;
  }

  void configure_post(std::string name, std::string &value)
  {
    value = post_tags_[name];
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

  void bind_environment(std::string name, environment_spec env)
  {
    nested_[name].e = std::make_unique<environment_spec>(env);
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

  void bind_tag_flow(std::pair<std::string, std::string> x,
                     std::pair<std::string, std::string> y)
  {
    tag_flow_constraints_.push_back({ x, y });
  }

  friend std::ostream &operator<<(std::ostream &out, const environment_spec &e)
  {
    out << term_colours::red_fg << "environment::" << e.name_
        << term_colours::reset << "\n";

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
    out << term_colours::yellow_fg << "pre-tags----" << term_colours::reset
        << "\n";
    for (auto [pre_tag, value] : e.pre_tags_)
      out << std::setw(26) << pre_tag << " : " << value << "\n";
    out << term_colours::yellow_fg << "post_tags----" << term_colours::reset
        << "\n";
    for (auto [post_tag, value] : e.post_tags_)
      out << std::setw(26) << post_tag << " : " << value << "\n";

    out << term_colours::yellow_fg << "nested----" << term_colours::reset
        << "\n";
    for (auto &[name, nspec] : e.nested_)
    {
      out << std::setw(26) << name << " :\n";
      if (nspec.pre_constraints.empty())
        out << "No pre-constraints\n";
      else
      {
        out << "pre-constraints:\n";
        for (auto name : nspec.pre_constraints)
          out << std::setw(26) << name << " :\n";
      }
      if (nspec.post_constraints.empty())
        out << "No post-constraints\n";
      else
      {
        out << "post-constraints:\n";
        for (auto name : nspec.post_constraints)
          out << std::setw(26) << name << " :\n";
      }
    }

    if (!e.tag_flow_constraints_.empty())
    {
      out << "with tag-flow-constraints:\n";
      for (auto name : e.tag_flow_constraints_)
        out << std::setw(26) << name.first.second << "(" << name.first.first
            << ") <=> " << name.second.second << "(" << name.second.first
            << ")\n";
    }
    return out;
  }
};

}   // namespace life
