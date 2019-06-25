
#include <iomanip>
#include <iostream>
#include <map>
#include <range/v3/all.hpp>
#include <regex>
#include <string>
#include <variant>

#include "../configuration.h"
#include "entity_spec.h"

namespace life {

io_signals
    entity_spec::instantiate_user_parameter_sizes(int sig_count)
{
  for (auto &n_sig : io_.inputs_)
    for (auto &[param, cp] : parameters_)
      if (cp.type_as_string() == "long" &&
          param == n_sig.second.user_parameter())
      {
        n_sig.second.instantiate_user_parameter(
             std::stol(cp.value_as_string()));
        n_sig.second.update_identifier("sig" + std::to_string(sig_count++));
      }
  for (auto &n_sig : io_.outputs_)
    for (auto &[param, cp] : parameters_)
      if (cp.type_as_string() == "long" &&
          param == n_sig.second.user_parameter())
      {
        n_sig.second.instantiate_user_parameter(
             std::stol(cp.value_as_string()));
        n_sig.second.update_identifier("sig" + std::to_string(sig_count++));
      }
  return io_;
}

entity_spec::entity_spec(parser p, block blk)
{

  auto t = life::all_entity_specs[blk.name_.substr(1)];

  *this = t;

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
      p.err_invalid_token(
          name, "override of " + name.expr_ + " must be of type entity");
      throw parser_error{};
    }

    auto f = ranges::find_if(
        nested_, [&](auto param) { return param.first == name.expr_; });
    if (f == nested_.end())
    {
      p.err_invalid_token(
          name, "this does not override any nested entitys of " + blk.name_);
      throw parser_error{};
    }

    f->second.e =
        std::make_unique<entity_spec>(life::entity_spec{ p, nested_blk });
  }
}

std::string
    entity_spec::dump(long depth)
{
  auto alignment = "\n" + std::string(depth, ' ');

  return alignment + "entity:" + name_ + alignment + "P" +
         (parameters_ | ranges::view::transform([&](auto parameter) {
            return alignment + parameter.first + ":" +
                   parameter.second.value_as_string();
          }) |
          ranges::action::join) +
         alignment + "I" +
         (io_.inputs_ | ranges::view::transform([&](auto sig) {
            return alignment + sig.second.full_name();
          }) |
          ranges::action::join) +
         alignment + "O" +
         (io_.outputs_ | ranges::view::transform([&](auto sig) {
            return alignment + sig.second.full_name();
          }) |
          ranges::action::join) +
         alignment + "n" +
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
    auto l = *f;
    auto p = l.find(':');
    io_.inputs_.push_back({ l.substr(0, p), signal_spec{ l } });
  }

  for (++f; *f != "n"; f++)
  {
    auto l = *f;
    auto p = l.find(':');
    io_.outputs_.push_back({ l.substr(0, p), signal_spec{ l } });
  }

  for (++f; f != pop_dump.end();)
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
  for (auto [input, value] : io_.inputs_)
    out << std::setw(26) << input << " : " << value.full_name() << "\n";
  out << term_colours::yellow_fg << "outputs----" << term_colours::reset
      << "\n";
  for (auto [output, value] : io_.outputs_)
    out << std::setw(26) << output << " : " << value.full_name() << "\n";
  return out.str();
}
}   // namespace life
