
#include <iomanip>
#include <iostream>
#include <map>
#include <range/v3/all.hpp>
#include <regex>
#include <string>
#include <variant>

#include "entity_spec.h"

namespace life {


  std::string entity_spec::dump(long depth)
  {
    auto alignment = "\n" + std::string(depth, ' ');
    return alignment + "entity:" + name_ +

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

           alignment + "n" +
           (nested_ | ranges::view::transform([&](auto nested) {
              return alignment + nested.first +
                     nested.second.e->dump(depth + 1);
            })   // | ranges::view::intersperse(";"+ alignment)
            | ranges::action::join);
  }

  std::string entity_spec::pretty_print()
  {
    std::stringstream out;
    out << term_colours::red_fg << "entity::" << name_ << term_colours::reset
        << "\n";

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
}
