
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <range/v3/all.hpp>
#include <regex>
#include <string>

#include "environment_spec.h"

namespace life {

std::string
    environment_spec::dump(long depth)
{
  auto alignment = "\n" + std::string(depth, ' ');

  auto pad = [&] {
    return ranges::view::transform(
               [&](auto p) { return alignment + p.first + ":" + p.second; }) |
           ranges::action::join;
  };

  return alignment + "environment:" + name_ + 
	     alignment + "P" +
         (parameters_ | ranges::view::transform([&](auto parameter) {
            return alignment + parameter.first + ":" +
                   parameter.second.value_as_string();
          }) |
          ranges::action::join) +
         alignment + "I" + (io_.inputs_ | pad()) + 
		 alignment + "O" + (io_.outputs_ | pad()) + 
		 alignment + "a" + (tags_ .pre_| pad()) +
         alignment + "b" + (tags_ .post_| pad()) + 
		 // needs to go
	     alignment + "r" +
         (traces_.pre_ | ranges::view::transform([&](auto trace) {
            return alignment + trace.trace_ + ":" +
                   trace.trace_;
          }) |
          ranges::action::join) +
	     alignment + "R" +
         (traces_.post_ | ranges::view::transform([&](auto trace) {
            return alignment + trace.trace_ + ":" +
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
    auto l                  = *f;
    auto p                  = l.find(':');
    io_.inputs_[l.substr(0, p)] = l.substr(p + 1);
  }

  for (++f; *f != "a"; f++)
  {
    auto l                   = *f;
    auto p                   = l.find(':');
    io_.outputs_[l.substr(0, p)] = l.substr(p + 1);
  }

  for (++f; *f != "b"; f++)
  {
    auto l                    = *f;
    auto p                    = l.find(':');
    tags_.pre_[l.substr(0, p)] = l.substr(p + 1);
  }

  for (++f; *f != "r"; f++)
  {
    auto l                    = *f;
    auto p                    = l.find(':');
    tags_.post_[l.substr(0, p)] = l.substr(p + 1);
  }

  for (++f; *f != "R"; f++)
  {
    auto l                    = *f;
    auto p                    = l.find(':');
    traces_.pre_.push_back({l.substr(0, p), std::stoi(l.substr(p + 1))});
  }

  for (++f; *f != "n"; f++)
  {
    auto l                     = *f;
    auto p                     = l.find(':');
    traces_.post_.push_back({l.substr(0, p), std::stoi(l.substr(p + 1))});
  }

  for (++f; f != pop_dump.end();)
  {
    auto p =
        std::find_if(f + 1, pop_dump.end(), [](auto l) { return l[0] != ' '; });

    std::transform(f + 1, p, f + 1, [](auto l) { return l.substr(1); });

    environment_spec e;
    nested_[*f].e = std::make_unique<environment_spec>(
        e.parse(std::vector<std::string>(f + 1, pop_dump.end())));

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
    out << std::setw(26) << input << " : " << value << "\n";
  out << term_colours::yellow_fg << "outputs----" << term_colours::reset
      << "\n";
  for (auto [output, value] : io_.outputs_)
    out << std::setw(26) << output << " : " << value << "\n";
  out << term_colours::yellow_fg << "pre-tags----" << term_colours::reset
      << "\n";
  for (auto [pre_tag, value] : tags_.pre_)
    out << std::setw(26) << pre_tag << " : " << value << "\n";
  out << term_colours::yellow_fg << "post_tags----" << term_colours::reset
      << "\n";
  for (auto [post_tag, value] : tags_.post_)
    out << std::setw(26) << post_tag << " : " << value << "\n";

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
}   // namespace life
