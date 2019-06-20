
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
#include "entity_spec.h"
#include "../parser/parser.h"

namespace life {

class population_spec {
  std::string                                    name_;
  std::map<std::string, configuration_primitive> parameters_;
  //std::map<std::string, std::string>             inputs_;
  //std::map<std::string, std::string>             outputs_;
  entity_spec                                    es_{ "null_entity" };

public:
  // population_spec() = default;

  population_spec(std::string name = "") : name_(name) {}
  population_spec(parser,block);

  auto name() const { return name_; }

  auto parameters() const { return parameters_; }

  //auto inputs() const { return inputs_; }

  //auto outputs() const { return outputs_; }

  template <typename T> void configure_parameter(std::string name, T &value)
  {
    parameters_[name].get_value(value);
  }

  template <typename T> void bind_parameter(std::string name, T value)
  {
    parameters_[name].set_value(value);
  }

  void bind_entity(entity_spec e) { es_ = e; }

  void configure_entity(entity_spec &e) { e = es_; }

  std::string dump(long depth);
  population_spec parse(std::vector<std::string> pop_dump);
  std::string pretty_print();

  //  friend std::ostream &operator<<(std::ostream &out, population_spec e)

  friend class environment_spec;
};

}   // namespace life
