
#include <iomanip>
#include <iostream>
#include <map>
#include <range/v3/all.hpp>
#include <regex>
#include <string>
#include <variant>

#include "../configuration.h"
#include "entity_spec.h"
#include "population_spec.h"

namespace life
{

population_spec::population_spec(parser p, block blk)
{

  *this = life::all_population_specs[blk.name_.substr(1)];

  for (auto over : blk.overrides_)
  {
    auto name  = over.first;
    auto value = over.second;

    auto f = ranges::find_if(
        parameters_, [&](auto param) { return param.first == name.expr_; });
    if (f == parameters_.end())
    {
      p.err_invalid_token(name,
                          "this does not override any parameters of " + name_);
      throw parser_error{};
    }

    configuration_primitive cp;
    cp.parse(value.expr_);
    if (cp.type_as_string() != f->second.type_as_string())
    {
      p.err_invalid_token(
          value, "type mismatch, should be " + f->second.type_as_string());
      throw parser_error{};
    }
    f->second = cp;
  }

  for (auto blover : blk.nested_)
  {
    auto name       = blover.first;
    auto nested_blk = blover.second;

    auto ct = config_manager::type_of_block(nested_blk.name_.substr(1));
    if (ct != "entity")
    {
      p.err_invalid_token(name,
                          "override of " + name.expr_ +
                              " inside population:: must be of type entity");
      throw parser_error{};
    }

    es_ = life::entity_spec{ p, nested_blk };
  }
}

std::string
    population_spec::dump(long depth)
{
  auto alignment = "\n" + std::string(depth, ' ');

  return alignment + "population:" + name_ + alignment + "P" +
         (parameters_ | ranges::view::transform([&](auto parameter) {
            return alignment + parameter.first + ":" +
                   parameter.second.value_as_string();
          }) |
          ranges::action::join) +
         alignment + "E" + es_.dump(depth + 1);
}

population_spec
    population_spec::parse(std::vector<std::string> pop_dump)
{
  name_ = *pop_dump.begin();
  name_ = name_.substr(name_.find(':') + 1);

  auto f = ranges::begin(pop_dump) + 2;

  for (; *f != "E"; f++)
  {
    auto                          l = *f;
    auto                          p = l.find(':');
    life::configuration_primitive c;
    c.parse(l.substr(p + 1));
    parameters_[l.substr(0, p)] = c;
  }

  for (auto l = (++f) + 1; l != pop_dump.end(); l++)
  {
    l->erase(0, 1);
  }

  entity_spec es;
  es_ = es.parse(std::vector<std::string>(f, pop_dump.end()));

  population_spec ps = *this;
  return ps;
}

std::string
    population_spec::pretty_print()
{
  std::stringstream out;
  out << "population::" << name_ << "\n{\n";

  out << " parameters\n";
  for (auto [parameter, value] : parameters_)
    out << std::setw(16) << parameter << " : " << value.value_as_string()
        << "\n";
  /*
out << term_colours::yellow_fg << "inputs----" << term_colours::reset
  << "\n";
for (auto [input, value] : inputs_)
out << std::setw(26) << input << " : " << value << "\n";
out << term_colours::yellow_fg << "outputs----" << term_colours::reset
  << "\n";
for (auto [output, value] : outputs_)
out << std::setw(26) << output << " : " << value << "\n";
    */

  out << "}\n";
  return out.str();
}
}   // namespace life
