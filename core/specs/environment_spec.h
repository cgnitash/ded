
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
    /*
            struct
     {
       std::vector<std::string> pre_;
       std::vector<std::string> post_;
     } constraints_;
         */
    Tags constraints_;
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

  void match_tags(SignalSpecSet &source_tags,
                  SignalSpecSet &sink_tags,
                  int &          tag_count);
  void update_and_match_tags(SignalSpecSet &source_tags,
                             SignalSpecSet &sink_tags,
                             int &          tag_count);
  void update_nested_constraints(SignalSpecSet &constraints);
  void match_tag_flow_equalities(int &tag_count);
  void match_nested_tag_constraints(int &tag_count);

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
      bind_parameter(
          std::string                                                 name,
          T                                                           value,
          std::vector<std::pair<std::function<bool(T)>, std::string>> cons = {})

  {
    if (parameters_.find(name) != parameters_.end())
    {
      std::cout << "User error: parameter " << name
                << " has already been declared\n";
      throw SpecError{};
    }
    parameters_[name].set_value(value);
	parameters_[name].set_constraints(cons);

  }

  template <typename T>
  void
      configure_parameter(std::string name, T &value)
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
    if (nested_.find(name) != nested_.end())
    {
      std::cout << "User error: nested environment " << name
                << " has already been declared\n";
      throw SpecError{};
    }
    nested_[name].e = std::make_unique<EnvironmentSpec>(env);
  }

  void
      configure_environment(std::string name, EnvironmentSpec &e)
  {
    if (nested_.find(name) == nested_.end())
    {
      std::cout << "User error: nested environment " << name
                << " has not been declared\n";
      throw SpecError{};
    }
    e = *nested_[name].e;
  }

  void
      bind_environment_pre_constraints(
          std::string                                      env_name,
          std::vector<std::pair<std::string, std::string>> pre_constraints)
  {
    // tags_.post_.push_back({ name, SignalSpec{ name, name, value } });
    nested_[env_name].constraints_.pre_ =
        pre_constraints |
        ranges::view::transform(
            [](auto tag) -> std::pair<std::string, SignalSpec> {
              auto name  = tag.first;
              auto value = tag.second;
              return { name, SignalSpec{ name, name, value } };
            });
  }

  void
      bind_environment_post_constraints(
          std::string                                      env_name,
          std::vector<std::pair<std::string, std::string>> post_constraints)
  {
    nested_[env_name].constraints_.post_ =
        post_constraints |
        ranges::view::transform(
            [](auto tag) -> std::pair<std::string, SignalSpec> {
              auto name  = tag.first;
              auto value = tag.second;
              return { name, SignalSpec{ name, name, value } };
            });
  }

  void
      bind_tag_equality(std::pair<std::string, std::string> x,
                        std::pair<std::string, std::string> y)
  {
    auto        is_pre_post = [](auto s) { return s == "pre" || s == "post"; };
    std::string error_message = "User error: cannot bind tag equality ";
    if (nested_.find(x.first) == nested_.end() ||
        nested_.find(y.first) == nested_.end() || !is_pre_post(x.second) ||
        !is_pre_post(y.second))
    {
      std::cout << "User error: " << error_message << "\n";
      throw SpecError{};
    }
	  
    tag_flow_equalities_.push_back({ x, y });
  }

  /*
  void
      bind_tag_inequality(std::pair<std::string, std::string> x,
                          std::pair<std::string, std::string> y)
  {
    tag_flow_inequalities_.push_back({ x, y });
  }
  */

  // friend std::ostream &operator<<(std::ostream &out, const EnvironmentSpec
  // &e)
  std::vector<std::string> dump(long depth, bool) const;
  EnvironmentSpec          parse(std::vector<std::string> pop_dump);

  std::string pretty_print();

  // void parser_parse();
  void                                       instantiate_user_parameter_sizes();
  void                                       bind_entity_io(IO);
  void                                       bind_tags(int);
  void                                       record_traces();
  std::vector<std::pair<Trace, std::string>> query_traces();

  friend class concepts::Environment;
};

}   // namespace specs
}   // namespace ded
