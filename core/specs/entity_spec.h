
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <range/v3/all.hpp>
#include <regex>
#include <string>
#include <variant>

#include "../term_colours.h"
#include "configuration_primitive.h"
#include "signal_spec.h"
#include "../parser/parser.h"

namespace life {
class entity_spec {
  struct nested_spec
  {
    std::unique_ptr<entity_spec> e;
    nested_spec() = default;
    nested_spec(const nested_spec &ns) : e(std::make_unique<entity_spec>(*ns.e))
    {
    }
  };

  std::string                                    name_;
  std::map<std::string, configuration_primitive> parameters_;
  std::map<std::string, nested_spec>             nested_;
  io_signals io_;

public:
  // entity_spec() = default;

  entity_spec(std::string name = "") : name_(name) {}
  entity_spec(parser,block);

  auto name() const { return name_; }

  /*
  auto parameters() const { return parameters_; }

  auto inputs() const { return inputs_; }

  auto outputs() const { return outputs_; }

  auto nested() const { return nested_; }
  */

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
    io_.inputs_.push_back({name, signal_spec{ name, name, value }});
  }

  void configure_input(std::string name, std::string &value)
  {
    auto i =
        ranges::find_if(io_.inputs_, [=](auto ns) { return ns.first == name; });
    value = i->second.id_type_specifier();
  }

  void bind_output(std::string name, std::string value)
  {
    io_.outputs_.push_back({name, signal_spec{ name, name, value }});
  }

  void configure_output(std::string name, std::string &value)
  {
    auto i =
        ranges::find_if(io_.outputs_, [=](auto ns) { return ns.first == name; });
    value = i->second.id_type_specifier();
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

  // friend std::ostream &operator<<(std::ostream &out, entity_spec e)
  std::string dump(long depth);
  entity_spec parse(std::vector<std::string> pop_dump);
  std::string pretty_print();

  io_signals instantiate_user_parameter_sizes();

};

}   // namespace life
