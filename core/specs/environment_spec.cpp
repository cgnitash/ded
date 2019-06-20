
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <range/v3/all.hpp>
#include <regex>
#include <string>

#include "environment_spec.h"
#include "../configuration.h"

namespace life {

	/*
void
    environment_spec::bind_all_entity_signals(population_spec ps)
{
  bind_all_entity_inputs(ps.es_.inputs());
  bind_all_entity_outputs(ps.es_.outputs());
}
	*/

environment_spec::environment_spec(parser p, block blk)
{

  *this = life::all_environment_specs.at(blk.name_.substr(1));

  for (auto over : blk.overrides_)
  {
    auto name  = over.first;
    auto value = over.second;

    auto f = ranges::find_if(parameters_, [&](auto param) {
      return param.first == name.expr_;
    });
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
	  if (ct != "environment")
    {
      p.err_invalid_token(
          name, "override of " + name.expr_ + " must be of type environment");
      throw parser_error{};
    }

    auto f = ranges::find_if(nested_, [&](auto param) {
      return param.first == name.expr_;
    });
    if (f == nested_.end())
    {
      p.err_invalid_token(
          name, "this does not override any nested environments " + blk.name_);
      throw parser_error{};
    }

    f->second.e = std::make_unique<environment_spec>(
        life::environment_spec{ p, nested_blk });
  }

}

std::string
    environment_spec::dump(long depth)
{
  auto alignment = "\n" + std::string(depth, ' ');

  /*
  auto pad = [&] {
    return ranges::view::transform(
               [&](auto p) { return alignment + p.first + ":" + p.second; }) |
           ranges::action::join;
  };
  */

  return alignment + "environment:" + name_ + alignment + "P" +
         (parameters_ | ranges::view::transform([&](auto parameter) {
            return alignment + parameter.first + ":" +
                   parameter.second.value_as_string();
          }) |
          ranges::action::join) +
         alignment + "I" +
         (io_.inputs_ | ranges::view::transform([&](auto sig) {
            return alignment + sig.second.full_name() ;
          }) |
          ranges::action::join) +
         alignment + "O" +
         (io_.outputs_ | ranges::view::transform([&](auto sig) {
            return alignment + sig.second.full_name() ;
          }) |
          ranges::action::join) +
         alignment + "a" +
         (tags_.pre_ | ranges::view::transform([&](auto sig) {
            return alignment + sig.second.full_name() ;
          }) |
          ranges::action::join) +
         alignment + "b" +
         (tags_.post_ | ranges::view::transform([&](auto sig) {
            return alignment + sig.second.full_name() ;
          }) |
          ranges::action::join) +
         // needs to go
         alignment + "r" +
         (traces_.pre_ | ranges::view::transform([&](auto trace) {
            return alignment + trace.signal_.full_name() +
                   std::to_string(trace.frequency_);
          }) |
          ranges::action::join) +
         alignment + "R" +
         (traces_.post_ | ranges::view::transform([&](auto trace) {
            return alignment + trace.signal_.full_name() +
                   std::to_string(trace.frequency_);
          }) |
          ranges::action::join) +
         // needs to go *
         alignment + "n" +
         (nested_ | ranges::view::transform([&](auto nested) {
            return alignment + nested.first + nested.second.e->dump(depth + 1);
          }) |
          ranges::action::join);
}

environment_spec
    environment_spec::parse(std::vector<std::string> pop_dump)
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
    auto l                      = *f;
    auto p                      = l.find(':');
    io_.inputs_[l.substr(0, p)] = signal_spec{l}; 
  }

  for (++f; *f != "a"; f++)
  {
    auto l                       = *f;
    auto p                       = l.find(':');
    io_.outputs_[l.substr(0, p)] = signal_spec{l}; 
  }

  for (++f; *f != "b"; f++)
  {
    auto l                     = *f;
    auto p                     = l.find(':');
    tags_.pre_[l.substr(0, p)] =  signal_spec{l};
  }

  for (++f; *f != "r"; f++)
  {
    auto l                      = *f;
    auto p                      = l.find(':');
    tags_.post_[l.substr(0, p)] =  signal_spec{l};
  }

  for (++f; *f != "R"; f++)
  {
    auto l = *f;
    auto p = l.find(';');
	signal_spec sp{l.substr(0, p)};
    traces_.pre_.push_back({sp, std::stoi(l.substr(p + 1)) });
  }

  for (++f; *f != "n"; f++)
  {
    auto l = *f;
    auto p = l.find(';');
	signal_spec sp{l.substr(0, p)};
    traces_.post_.push_back({ sp , std::stoi(l.substr(p + 1)) });
  }

  for (++f; f != pop_dump.end();)
  {
    auto p = std::find_if(
        f + 1, pop_dump.end(), [](auto l) { return l[0] != ' '; });

    std::transform(f + 1, p, f + 1, [](auto l) { return l.substr(1); });

    environment_spec e;
	e.set_user_specified_name(*f);
    nested_[*f].e = std::make_unique<environment_spec>(
        e.parse(std::vector<std::string>(f + 1, p)));

    f = p;
  }
  return *this;
}

std::string
    environment_spec::pretty_print()
{
  std::stringstream out;
  out << term_colours::red_fg << "environment::" << name_ << term_colours::reset
      << "\n";

  out << term_colours::yellow_fg << "parameters----" << term_colours::reset
      << "\n";
  for (auto [parameter, value] : parameters_)
    out << std::setw(26) << parameter << " : " << value.value_as_string()
        << "\n";
  out << term_colours::yellow_fg << "inputs----" << term_colours::reset << "\n";
  for (auto [input, value] : io_.inputs_)
    out << std::setw(26) << input << " : " << value.full_name() << "\n";
  out << term_colours::yellow_fg << "outputs----" << term_colours::reset
      << "\n";
  for (auto [output, value] : io_.outputs_)
    out << std::setw(26) << output << " : " << value.full_name() << "\n";
  out << term_colours::yellow_fg << "pre-tags----" << term_colours::reset
      << "\n";
  for (auto [pre_tag, value] : tags_.pre_)
    out << std::setw(26) << pre_tag << " : " << value.full_name() << "\n";
  out << term_colours::yellow_fg << "post_tags----" << term_colours::reset
      << "\n";
  for (auto [post_tag, value] : tags_.post_)
    out << std::setw(26) << post_tag << " : " << value.full_name() << "\n";

  out << term_colours::yellow_fg << "nested----" << term_colours::reset << "\n";
  for (auto &[name, nspec] : nested_)
  {
    out << std::setw(26) << name << " :\n";
    if (nspec.constraints_.pre_.empty())
      out << "No pre-constraints\n";
    else
    {
      out << "pre-constraints:\n";
      for (auto name : nspec.constraints_.pre_)
        out << std::setw(26) << name << " :\n";
    }
    if (nspec.constraints_.post_.empty())
      out << "No post-constraints\n";
    else
    {
      out << "post-constraints:\n";
      for (auto name : nspec.constraints_.post_)
        out << std::setw(26) << name << " :\n";
    }
  }

  if (!tag_flow_equalities.empty() || !tag_flow_inequalities.empty())
  {
    out << "with tag-flow-constraints:\n";
    for (auto name : tag_flow_equalities)
      out << std::setw(26) << name.first.second << "(" << name.first.first
          << ") <=> " << name.second.second << "(" << name.second.first
          << ")\n";
    for (auto name : tag_flow_inequalities)
      out << std::setw(26) << name.first.second << "(" << name.first.first
          << ") <!=> " << name.second.second << "(" << name.second.first
          << ")\n";
  }
  return out.str();
}
}   // namespace life
