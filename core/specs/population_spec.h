
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

namespace life {
class population_spec {
  std::string                                    name_;
  std::map<std::string, configuration_primitive> parameters_;
  std::map<std::string, std::string>             inputs_;
  std::map<std::string, std::string>             outputs_;
  entity_spec                                    es_{ "null_entity" };

public:
  // population_spec() = default;

  population_spec(std::string name = "") : name_(name) {}

  auto name() const { return name_; }

  auto parameters() const { return parameters_; }

  auto inputs() const { return inputs_; }

  auto outputs() const { return outputs_; }

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

  //  friend std::ostream &operator<<(std::ostream &out, population_spec e)

  std::string dump(long depth)
  {
    auto alignment = "\n" + std::string(depth, ' ');
    return alignment + "population:" + name_ +

           alignment + "P" +
           (parameters_ | ranges::view::transform([&](auto parameter) {
              return alignment + parameter.first + ":" +
                     parameter.second.value_as_string();
            }) |
            ranges::action::join) +

           alignment + "I" +
           (inputs_ | ranges::view::transform([&](auto input) {
              return alignment + input.first + ":" + input.second;
            })   //|            ranges::view::intersperse(";"+ alignment)
            | ranges::action::join) +

           alignment + "O" +
           (outputs_ | ranges::view::transform([&](auto output) {
              return alignment + output.first + ":" + output.second;
            })   // | ranges::view::intersperse(";"+ alignment)
            | ranges::action::join) +

           alignment + "E" + es_.dump(depth + 1);
  }

  population_spec parse(std::string pop_dump)
  {
    population_spec ps;
    return ps;
  }

  std::string pretty_print()
  {
    std::stringstream out;
    out << term_colours::red_fg << "population::" << name_
        << term_colours::reset << "\n";

    out << term_colours::yellow_fg << "parameters----" << term_colours::reset
        << "\n";
    for (auto [parameter, value] : parameters_)
      out << std::setw(26) << parameter << " : " << value.value_as_string()
          << "\n";
    out << term_colours::yellow_fg << "inputs----" << term_colours::reset
        << "\n";
    for (auto [input, value] : inputs_)
      out << std::setw(26) << input << " : " << value << "\n";
    out << term_colours::yellow_fg << "outputs----" << term_colours::reset
        << "\n";
    for (auto [output, value] : outputs_)
      out << std::setw(26) << output << " : " << value << "\n";

    return out.str();
  }
};

}   // namespace life
