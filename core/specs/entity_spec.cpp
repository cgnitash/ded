
#include <iomanip>
#include <iostream>
#include <map>
#include <range/v3/all.hpp>
#include <regex>
#include <string>
#include <variant>

#include "entity_spec.h"

namespace life {

entity_spec::entity_spec(parser , block )
{
}

std::string
    entity_spec::dump(long depth)
{
  auto alignment = "\n" + std::string(depth, ' ');

  auto pad = [&] {
    return ranges::view::transform(
               [&](auto p) { return alignment + p.first + ":" + p.second; }) |
           ranges::action::join;
  };

  return alignment + "entity:" + name_ + alignment + "P" +
         (parameters_ | ranges::view::transform([&](auto parameter) {
            return alignment + parameter.first + ":" +
                   parameter.second.value_as_string();
          }) |
          ranges::action::join) +
         alignment + "I" + (inputs_ | pad()) + alignment + "O" +
         (outputs_ | pad()) + alignment + "n" +
         (nested_ | ranges::view::transform([&](auto nested) {
            return alignment + nested.first + nested.second.e->dump(depth + 1);
          }) |
          ranges::action::join);
}

entity_spec
    entity_spec::parse(std::vector<std::string> pop_dump)
{
  name_ = *pop_dump.begin();
  name_ = name_.substr(name_.find(':') + 1);

  auto f = ranges::begin(pop_dump) + 2;

  for (; *f != "I"; f++)
  {
    auto                          l = *f;
    auto                          p = l.find(':');
    life::configuration_primitive c;
    c.parse(l.substr(p + 1));
    parameters_[l.substr(0, p)] = c;
  }

  for (++f; *f != "O"; f++)
  {
    auto l                  = *f;
    auto p                  = l.find(':');
    inputs_[l.substr(0, p)] = l.substr(p + 1);
  }

  for (++f; *f != "n"; f++)
  {
    auto l                   = *f;
    auto p                   = l.find(':');
    outputs_[l.substr(0, p)] = l.substr(p + 1);
  }

  for (++f; f != pop_dump.end(); )
  {
    auto p =
        std::find_if(f + 1, pop_dump.end(), [](auto l) { return l[0] != ' '; });

    std::transform(f + 1, p, f + 1, [](auto l) { return l.substr(1); });

    entity_spec e;
    nested_[*f].e = std::make_unique<entity_spec>(
        e.parse(std::vector<std::string>(f + 1, pop_dump.end())));

	f = p;

  }

    // entity_spec ps = *this;
    return *this;
  }

std::string
    entity_spec::pretty_print()
{
  std::stringstream out;
  out << term_colours::red_fg << "entity::" << name_ << term_colours::reset
      << "\n";

  out << term_colours::yellow_fg << "parameters----" << term_colours::reset
      << "\n";
  for (auto [parameter, value] : parameters_)
    out << std::setw(26) << parameter << " : " << value.value_as_string()
        << "\n";
  out << term_colours::yellow_fg << "inputs----" << term_colours::reset << "\n";
  for (auto [input, value] : inputs_)
    out << std::setw(26) << input << " : " << value << "\n";
  out << term_colours::yellow_fg << "outputs----" << term_colours::reset
      << "\n";
  for (auto [output, value] : outputs_)
    out << std::setw(26) << output << " : " << value << "\n";
  return out.str();
}
}   // namespace life
