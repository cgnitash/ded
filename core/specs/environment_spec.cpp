
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <range/v3/all.hpp>
#include <regex>
#include <string>

#include "environment_spec.h"

namespace life {

  std::string environment_spec::dump(long depth)
  {
    auto alignment = "\n" + std::string(depth, ' ');

    auto pad = [&] {
      return ranges::view::transform(
                 [&](auto p) { return alignment + p.first + ":" + p.second; }) |
             ranges::action::join;
    };

    return alignment + "environment:" + name_ + alignment + "P" +
           (parameters_ | ranges::view::transform([&](auto parameter) {
              return alignment + parameter.first + ":" +
                     parameter.second.value_as_string();
            }) |
            ranges::action::join) +
           alignment + "I" + (inputs_ | pad()) + alignment + "O" +
           (outputs_ | pad()) + alignment + "a" + (pre_tags_ | pad()) +
           alignment + "b" + (post_tags_ | pad()) + alignment + "n" +
           (nested_ | ranges::view::transform([&](auto nested) {
              return alignment + nested.first +
                     nested.second.e->dump(depth + 1);
            }) |
            ranges::action::join);
  }

  std::string environment_spec::pretty_print()
  {
    std::stringstream out;
    out << term_colours::red_fg << "environment::" << name_
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
    out << term_colours::yellow_fg << "pre-tags----" << term_colours::reset
        << "\n";
    for (auto [pre_tag, value] : pre_tags_)
      out << std::setw(26) << pre_tag << " : " << value << "\n";
    out << term_colours::yellow_fg << "post_tags----" << term_colours::reset
        << "\n";
    for (auto [post_tag, value] : post_tags_)
      out << std::setw(26) << post_tag << " : " << value << "\n";

    out << term_colours::yellow_fg << "nested----" << term_colours::reset
        << "\n";
    for (auto &[name, nspec] : nested_)
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

    if (!tag_flow_constraints_.empty())
    {
      out << "with tag-flow-constraints:\n";
      for (auto name : tag_flow_constraints_)
        out << std::setw(26) << name.first.second << "(" << name.first.first
            << ") <=> " << name.second.second << "(" << name.second.first
            << ")\n";
    }
    return out.str();
  }
	}
