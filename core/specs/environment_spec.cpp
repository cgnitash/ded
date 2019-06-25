
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <range/v3/all.hpp>
#include <regex>
#include <string>

#include "../configuration.h"
#include "environment_spec.h"

namespace life {

void
    environment_spec::bind_tags(int tag_count)
{

  for (auto [source, sink] : tag_flow_equalities_)
  {
    auto &source_tags = source.second == "pre"
                            ? nested_[source.first].e->tags_.pre_
                            : nested_[source.first].e->tags_.post_;
    auto &sink_tags = sink.second == "pre" ? nested_[sink.first].e->tags_.pre_
                                           : nested_[sink.first].e->tags_.post_;
    if (source_tags.size() != sink_tags.size())
    {
      std::cout << "cannot match flow equality\n";
      // throw;
    }
    auto sink_tags_copy = sink_tags;
    for (auto &n_tag : source_tags)
    {
      auto &src     = n_tag.second;
      auto  matches = ranges::count_if(sink_tags_copy, [sig = src](auto ns) {
        return ns.second.exactly_matches(sig);
      });
      if (matches > 1)
      {
        std::cout << "error: multiple tags match exactly\n";
        // throw;
      }
      if (!matches)
      {
        std::cout << "error: no tags match exactly (convertible signals "
                     "not supported yet)\n";
        // throw;
      }

      ranges::find_if(
          sink_tags,
          [sig = src](auto ns) { return ns.second.exactly_matches(sig); })
          ->second.update_identifier("tag" + std::to_string(tag_count));

      src.update_identifier("tag" + std::to_string(tag_count));

      sink_tags_copy.erase(
          ranges::find_if(sink_tags_copy, [sig = src](auto ns) {
            return ns.second.exactly_matches(sig);
          }));

      tag_count++;
    }
  }

  for (auto &es : nested_) es.second.e->bind_tags(tag_count);
}

void
    environment_spec::instantiate_user_parameter_sizes()
{
  for (auto &n_sig : io_.inputs_)
    for (auto &[param, cp] : parameters_)
      if (cp.type_as_string() == "long" &&
          param == n_sig.second.user_parameter())
        n_sig.second.instantiate_user_parameter(
            std::stol(cp.value_as_string()));

  for (auto &n_sig : io_.outputs_)
    for (auto &[param, cp] : parameters_)
      if (cp.type_as_string() == "long" &&
          param == n_sig.second.user_parameter())
        n_sig.second.instantiate_user_parameter(
            std::stol(cp.value_as_string()));

  for (auto &es : nested_) es.second.e->instantiate_user_parameter_sizes();
}

void
    environment_spec::bind_entity_io(io_signals ios)
{
  for (auto &es : nested_) es.second.e->bind_entity_io(ios);

  for (auto &n_sig : io_.inputs_)
  {
    auto &in_sig  = n_sig.second;
    auto  matches = ranges::count_if(ios.inputs_, [sig = in_sig](auto ns) {
      return ns.second.exactly_matches(sig);
    });
    if (matches > 1)
    {
      std::cout << "error: multiple input signals match exactly\n";
      // throw;
    }
    if (!matches)
    {
      std::cout << "error: no input signals match exactly (convertible signals "
                   "not supported yet)\n";
      // throw;
    }
    auto i = ranges::find_if(ios.inputs_, [sig = in_sig](auto ns) {
      return ns.second.exactly_matches(sig);
    });
    in_sig.update_identifier(i->second.identifier());
    ios.inputs_.erase(i);
  }

  for (auto &n_sig : io_.outputs_)
  {
    auto &out_sig = n_sig.second;
    auto  matches = ranges::count_if(ios.outputs_, [sig = out_sig](auto ns) {
      return ns.second.exactly_matches(sig);
    });
    if (matches > 1)
    {
      std::cout << "error: multiple output signals match exactly\n";
      // throw;
    }
    if (!matches)
    {
      std::cout << "error: no output signals match exactly (convertible "
                   "signals not supported yet)\n";
      // throw;
    }
    auto i = ranges::find_if(ios.outputs_, [sig = out_sig](auto ns) {
      return ns.second.exactly_matches(sig);
    });
    out_sig.update_identifier(i->second.identifier());
    ios.outputs_.erase(i);
  }
}

void
    environment_spec::record_traces()
{
  for (auto &[sig, freq] : traces_.pre_)
    sig.update_identifier(ranges::find_if(tags_.pre_,
                                          [n = sig.user_name()](auto tag) {
                                            return tag.first == n;
                                          })
                              ->second.identifier());

  for (auto &[sig, freq] : traces_.post_)
    sig.update_identifier(ranges::find_if(tags_.post_,
                                          [n = sig.user_name()](auto tag) {
                                            return tag.first == n;
                                          })
                              ->second.identifier());

  for (auto &es : nested_) es.second.e->record_traces();
}

environment_spec::environment_spec(parser parser_, block block_)
{

  *this = life::all_environment_specs.at(block_.name_.substr(1));

  for (auto over : block_.overrides_)
  {
    auto name  = over.first;
    auto value = over.second;

    auto f = ranges::find_if(
        parameters_, [&](auto param) { return param.first == name.expr_; });
    if (f == parameters_.end())
    {
      parser_.err_invalid_token(
          name, "this does not override any parameters of " + name_);
      throw parser_error{};
    }

    configuration_primitive cp;
    cp.parse(value.expr_);
    if (cp.type_as_string() != f->second.type_as_string())
    {
      parser_.err_invalid_token(
          value, "type mismatch, should be " + f->second.type_as_string());
      throw parser_error{};
    }
    f->second = cp;
  }

  for (auto over : block_.traces_)
  {
    auto tag_name  = over.first;
    auto frequency = over.second;

    configuration_primitive cp;
    cp.parse(frequency.expr_);
    if (cp.type_as_string() != "long")
    {
      parser_.err_invalid_token(frequency,
                                "expected frequency of trace (number) here");
      throw parser_error{};
    }

    if (auto i = ranges::find_if(
            tags_.pre_,
            [name = tag_name.expr_](auto ns) { return ns.first == name; });
        i != ranges::end(tags_.pre_))
    {
      traces_.pre_.push_back(
          { i->second.full_name(), std::stoi(frequency.expr_) });
    } else if (auto j = ranges::find_if(tags_.post_,
                                        [name = tag_name.expr_](auto ns) {
                                          return ns.first == name;
                                        });
               i != ranges::end(tags_.post_))
    {
      traces_.post_.push_back(
          { j->second.full_name(), std::stoi(frequency.expr_) });
    } else
    {
      parser_.err_invalid_token(tag_name,
                                "this is not a pre/post tag of " + name_);
      throw parser_error{};
    }
  }

  for (auto blover : block_.nested_)
  {
    auto name       = blover.first;
    auto nested_blk = blover.second;

    auto ct = config_manager::type_of_block(nested_blk.name_.substr(1));
    if (ct != "environment")
    {
      parser_.err_invalid_token(
          name, "override of " + name.expr_ + " must be of type environment");
      throw parser_error{};
    }

    auto f = ranges::find_if(
        nested_, [&](auto param) { return param.first == name.expr_; });
    if (f == nested_.end())
    {
      parser_.err_invalid_token(
          name,
          "this does not override any nested environments " + block_.name_);
      throw parser_error{};
    }

    f->second.e = std::make_unique<environment_spec>(
        life::environment_spec{ parser_, nested_blk });
    f->second.e->set_user_specified_name(name.expr_);
  }
}

std::vector<std::string>
    environment_spec::dump(long depth)
{
  std::vector<std::string> lines;
  // auto alignment = "\n" + std::string(depth, ' ');
  auto alignment = std::string(depth, ' ');

  /*
  auto pad = [&] {
    return ranges::view::transform(
               [&](auto p) { return alignment + p.first + ":" + p.second; }) |
           ranges::action::join;
  };
  */
  auto pad_signal = [&](auto sig) {
    return alignment + sig.second.full_name();
  };

  lines.push_back(alignment + "environment:" + name_);
  lines.push_back(alignment + "P");
  ranges::transform(
      parameters_, ranges::back_inserter(lines), [&](auto parameter) {
        return alignment + parameter.first + ":" +
               parameter.second.value_as_string();
      });
  lines.push_back(alignment + "I");
  ranges::transform(io_.inputs_, ranges::back_inserter(lines), pad_signal);
  lines.push_back(alignment + "O");
  ranges::transform(io_.outputs_, ranges::back_inserter(lines), pad_signal);
  lines.push_back(alignment + "a");
  ranges::transform(tags_.pre_, ranges::back_inserter(lines), pad_signal);
  lines.push_back(alignment + "b");
  ranges::transform(tags_.post_, ranges::back_inserter(lines), pad_signal);
  // needs to go
  lines.push_back(alignment + "r");
  ranges::transform(
      traces_.pre_, ranges::back_inserter(lines), [&](auto trace) {
        return alignment + trace.signal_.full_name() + ";" +
               std::to_string(trace.frequency_);
      });
  lines.push_back(alignment + "R");
  ranges::transform(
      traces_.post_, ranges::back_inserter(lines), [&](auto trace) {
        return alignment + trace.signal_.full_name() + ";" +
               std::to_string(trace.frequency_);
      });
  // needs to go *
  lines.push_back(alignment + "n");
  for (auto nested : nested_)
  {
    lines.push_back(alignment + nested.first);
    auto n_dump = nested.second.e->dump(depth + 1);
    lines.insert(lines.end(), n_dump.begin(), n_dump.end());
  }

  return lines;
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
    auto l = *f;
    auto p = l.find(':');
    io_.inputs_.push_back({ l.substr(0, p), signal_spec{ l } });
  }

  for (++f; *f != "a"; f++)
  {
    auto l = *f;
    auto p = l.find(':');
    io_.outputs_.push_back({ l.substr(0, p), signal_spec{ l } });
  }

  for (++f; *f != "b"; f++)
  {
    auto l = *f;
    auto p = l.find(':');
    tags_.pre_.push_back({ l.substr(0, p), signal_spec{ l } });
  }

  for (++f; *f != "r"; f++)
  {
    auto l = *f;
    auto p = l.find(':');
    tags_.post_.push_back({ l.substr(0, p), signal_spec{ l } });
  }

  for (++f; *f != "R"; f++)
  {
    auto l = *f;
    auto p = l.find(';');
    traces_.pre_.push_back({ l.substr(0, p - 1), std::stoi(l.substr(p + 1)) });
  }

  for (++f; *f != "n"; f++)
  {
    auto l = *f;
    auto p = l.find(';');
    traces_.post_.push_back({ l.substr(0, p - 1), std::stoi(l.substr(p + 1)) });
  }

  for (++f; f != pop_dump.end();)
  {
    auto p =
        std::find_if(f + 1, pop_dump.end(), [](auto l) { return l[0] != ' '; });

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

  if (!tag_flow_equalities_.empty() || !tag_flow_inequalities_.empty())
  {
    out << "with tag-flow-constraints:\n";
    for (auto name : tag_flow_equalities_)
      out << std::setw(26) << name.first.second << "(" << name.first.first
          << ") <=> " << name.second.second << "(" << name.second.first
          << ")\n";
    for (auto name : tag_flow_inequalities_)
      out << std::setw(26) << name.first.second << "(" << name.first.first
          << ") <!=> " << name.second.second << "(" << name.second.first
          << ")\n";
  }
  return out.str();
}
}   // namespace life
