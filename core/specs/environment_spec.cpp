
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <string>

#include "../configuration.h"
#include "environment_spec.h"

namespace ded
{
namespace specs
{

void
    EnvironmentSpec::matchTags(SignalSpecSet &source_tags,
                                SignalSpecSet &sink_tags,
                                int &          tag_count)
{

  if (source_tags.size() != sink_tags.size())
  {
    std::cout << "cannot match flow equality\n";
    throw SpecError{};
  }
  auto sink_tags_copy = sink_tags;
  for (auto &n_tag : source_tags)
  {
    auto &src     = n_tag.second;
    auto  matches = rs::count_if(sink_tags_copy, [sig = src](auto ns) {
      return ns.second.exactlyMatches(sig);
    });
    if (matches > 1)
    {
      std::cout << "error: multiple tags match exactly\n";
      throw SpecError{};
    }
    if (!matches)
    {
      std::cout << "error: no tags match exactly (convertible signals "
                   "not supported yet)\n";
      throw SpecError{};
    }

    auto snk = rs::find_if(sink_tags, [sig = src](auto ns) {
      return ns.second.exactlyMatches(sig);
    });

    if (snk->second.identifier()[0] == '~')
    {
      if (src.identifier()[0] != '~')
        src.updateIdentifier(snk->second.identifier());
      if (snk->second.identifier() != src.identifier())
      {
        std::cout
            << "error: tags were previously bound, and are in conflict now\n";
        throw SpecError{};
      }
    }
    else if (src.identifier()[0] == '~')
    {
      if (snk->second.identifier()[0] != '~')
        snk->second.updateIdentifier(src.identifier());
      if (snk->second.identifier() != src.identifier())
      {
        std::cout << "error: tags were previously bound, and are in "
                     "conflict now\n";
        throw SpecError{};
      }
    }
    else
    {
      tag_count++;
      snk->second.updateIdentifier("~tag" + std::to_string(tag_count));
      src.updateIdentifier("~tag" + std::to_string(tag_count));
    }

    sink_tags_copy.erase(rs::find_if(sink_tags_copy, [sig = src](auto ns) {
      return ns.second.exactlyMatches(sig);
    }));
  }
}

void
    EnvironmentSpec::matchTagFlowEqualities(int &tag_count)
{

  for (auto [source, sink] : tag_flow_equalities_)
  {
    auto &source_tags = source.second == "pre"
                            ? nested_[source.first].e->tags_.pre_
                            : nested_[source.first].e->tags_.post_;
    auto &sink_tags = sink.second == "pre" ? nested_[sink.first].e->tags_.pre_
                                           : nested_[sink.first].e->tags_.post_;
    matchTags(source_tags, sink_tags, tag_count);
  }
}

void
    EnvironmentSpec::updateNestedConstraints(SignalSpecSet &constraints)
{
  for (auto &source_tag : constraints)
  {
    auto same_user_name = [&source_tag](auto tag) {
      return tag.second.userName() == source_tag.second.userName();
    };
    if (auto f = rs::find_if(tags_.pre_, same_user_name);
        f != rs::end(tags_.pre_))
      source_tag.second.updateIdentifier(f->second.identifier());
    else if (f = rs::find_if(tags_.post_, same_user_name);
             f != rs::end(tags_.post_))
      source_tag.second.updateIdentifier(f->second.identifier());
  }
}

void
    EnvironmentSpec::updateAndMatchTags(SignalSpecSet &source_tags,
                                           SignalSpecSet &sink_tags,
                                           int &          tag_count)
{
  if (!source_tags.empty())
  {
    updateNestedConstraints(source_tags);
    matchTags(source_tags, sink_tags, tag_count);
  }
}

void
    EnvironmentSpec::matchNestedTagConstraints(int &tag_count)
{
  for (auto &es : nested_)
  {
    updateAndMatchTags(
        es.second.constraints_.pre_, es.second.e->tags_.pre_, tag_count);
    updateAndMatchTags(
        es.second.constraints_.post_, es.second.e->tags_.post_, tag_count);
  }
}
void
    EnvironmentSpec::bindTags(int tag_count)
{

  matchTagFlowEqualities(tag_count);

  matchNestedTagConstraints(tag_count);

  for (auto &es : nested_)
    es.second.e->bindTags(tag_count);
}

void
    EnvironmentSpec::instantiateUserParameterSizes()
{
  for (auto &n_sig : io_.inputs_)
    for (auto &[param, cp] : parameters_)
      if (cp.typeAsString() == "long" &&
          param == n_sig.second.userParameter())
        n_sig.second.instantiateUserParameter(
            std::stol(cp.valueAsString()));

  for (auto &n_sig : io_.outputs_)
    for (auto &[param, cp] : parameters_)
      if (cp.typeAsString() == "long" &&
          param == n_sig.second.userParameter())
        n_sig.second.instantiateUserParameter(
            std::stol(cp.valueAsString()));

  for (auto &es : nested_)
    es.second.e->instantiateUserParameterSizes();
}

void
    EnvironmentSpec::bindEntityIO(IO ios)
{
  for (auto &es : nested_)
    es.second.e->bindEntityIO(ios);

  for (auto &n_sig : io_.inputs_)
  {
    auto &in_sig  = n_sig.second;
    auto  matches = rs::count_if(ios.inputs_, [sig = in_sig](auto ns) {
      return ns.second.exactlyMatches(sig);
    });
    if (matches > 1)
    {
      std::cout << "error: multiple input signals match exactly\n";
      // throw;
    }
    if (!matches)
    {
      std::cout << "error: no input signals match exactly (convertible signals "
                   "not supported yet)\n  "
                << n_sig.second.fullName() << "\nviable candidates";
      for (auto sig : ios.inputs_)
        std::cout << "\n    " << sig.second.fullName();
      throw language::ParserError{};
    }
    auto i = rs::find_if(ios.inputs_, [sig = in_sig](auto ns) {
      return ns.second.exactlyMatches(sig);
    });
    in_sig.updateIdentifier(i->second.identifier());
    ios.inputs_.erase(i);
  }

  for (auto &n_sig : io_.outputs_)
  {
    auto &out_sig = n_sig.second;
    auto  matches = rs::count_if(ios.outputs_, [sig = out_sig](auto ns) {
      return ns.second.exactlyMatches(sig);
    });
    if (matches > 1)
    {
      std::cout << "error: multiple output signals match exactly\n";
      // throw;
    }
    if (!matches)
    {
      std::cout << "error: no input signals match exactly (convertible signals "
                   "not supported yet)\n  "
                << n_sig.second.fullName() << "\nviable candidates";
      for (auto sig : ios.outputs_)
        std::cout << "\n    " << sig.second.fullName();
      throw language::ParserError{};
    }
    auto i = rs::find_if(ios.outputs_, [sig = out_sig](auto ns) {
      return ns.second.exactlyMatches(sig);
    });
    out_sig.updateIdentifier(i->second.identifier());
    ios.outputs_.erase(i);
  }
}

std::vector<std::pair<Trace, std::string>>
    EnvironmentSpec::queryTraces()
{

  std::vector<std::pair<Trace, std::string>> res;

  for (auto &sig_freq : traces_.pre_)
    res.push_back({ sig_freq, {} });

  for (auto &sig_freq : traces_.post_)
    res.push_back({ sig_freq, {} });

  for (auto &es : nested_)
    for (auto ts : es.second.e->queryTraces())
      res.push_back(ts);

  for (auto &r : res)
    r.second = user_specified_name_ + "_" + name_ + "/" + r.second;

  return res;
}

void
    EnvironmentSpec::recordTraces()
{

  for (auto &sig_freq : traces_.pre_)
    sig_freq.signal_.updateIdentifier(
        rs::find_if(tags_.pre_,
                        [n = sig_freq.signal_.userName()](auto tag) {
                          return tag.first == n;
                        })
            ->second.identifier());

  for (auto &sig_freq : traces_.post_)
    sig_freq.signal_.updateIdentifier(
        rs::find_if(tags_.post_,
                        [n = sig_freq.signal_.userName()](auto tag) {
                          return tag.first == n;
                        })
            ->second.identifier());

  for (auto &es : nested_)
    es.second.e->recordTraces();
}

EnvironmentSpec::EnvironmentSpec(language::Parser parser_,
                                 language::Block  block_)
{

  *this = all_environment_specs.at(block_.name_.substr(1));

  for (auto over : block_.overrides_)
  {
    auto name  = over.first;
    auto value = over.second;

    auto f = rs::find_if(
        parameters_, [&](auto param) { return param.first == name.expr_; });
    if (f == parameters_.end())
    {
      parser_.errInvalidToken(
          name,
          "this does not override any parameters of " + name_,
          parameters_ |
              rv::transform([](auto param) { return param.first; }));
      throw language::ParserError{};
    }

    ConfigurationPrimitive cp;
    cp.parse(value.expr_);
    if (cp.typeAsString() != f->second.typeAsString())
    {
      using namespace std::literals::string_literals;
      parser_.errInvalidToken(
          value,
          (cp.typeAsString() == "NULL"
               ? "unable to parse configuration primitive"s
               : "type mismatch"s) +
              ", should be '" + f->second.typeAsString() + "'");
      throw language::ParserError{};
    }
    f->second.parse(cp.valueAsString());
    auto con = f->second.checkConstraints();
    if (con)
    {
      parser_.errInvalidToken(
          value,
              "parameter constraint not satisfied: " + *con );
      throw language::ParserError{};
    }
  }

  for (auto over : block_.traces_)
  {
    auto tag_name  = over.first;
    auto frequency = over.second;

    ConfigurationPrimitive cp;
    cp.parse(frequency.expr_);
    if (cp.typeAsString() != "long")
    {
      parser_.errInvalidToken(frequency,
                                "expected frequency of trace (number) here");
      throw language::ParserError{};
    }

    if (auto i = rs::find_if(
            tags_.pre_,
            [name = tag_name.expr_](auto ns) { return ns.first == name; });
        i != rs::end(tags_.pre_))
    {
      traces_.pre_.push_back(
          { i->second.fullName(), std::stoi(frequency.expr_) });
    }
    else if (i = rs::find_if(
                 tags_.post_,
                 [name = tag_name.expr_](auto ns) { return ns.first == name; });
             i != rs::end(tags_.post_))
    {
      traces_.post_.push_back(
          { i->second.fullName(), std::stoi(frequency.expr_) });
    }
    else
    {
      parser_.errInvalidToken(
          tag_name,
          "this is not a pre/post tag of " + name_,
          rv::concat(
              tags_.pre_ |
                  rv::transform([](auto tag) { return tag.first; }),
              tags_.post_ |
                  rv::transform([](auto tag) { return tag.first; })));
      throw language::ParserError{};
    }
  }

  for (auto blover : block_.nested_)
  {
    auto name       = blover.first;
    auto nested_blk = blover.second;

    auto f = rs::find_if(
        nested_, [&](auto param) { return param.first == name.expr_; });
    if (f == nested_.end())
    {
      parser_.errInvalidToken(
          name,
          "this does not override any nested environments " + block_.name_,
          nested_ |
              rv::transform([](auto param) { return param.first; }));
      throw language::ParserError{};
    }

    auto ct = config_manager::typeOfBlock(nested_blk.name_.substr(1));
    if (ct != "environment")
    {
      parser_.errInvalidToken(name,
                                "override of " + name.expr_ +
                                    " must be of type environment",
                                config_manager::allEnvironmentNames());
      throw language::ParserError{};
    }

    f->second.e = std::make_unique<EnvironmentSpec>(
        EnvironmentSpec{ parser_, nested_blk });
    f->second.e->setUserSpecifiedName(name.expr_);
  }
}

std::vector<std::string>
    EnvironmentSpec::dump(long depth, bool with_traces) const
{
  std::vector<std::string> lines;
  auto                     alignment = std::string(depth, ' ');

  auto pad_signal = [&](auto sig) {
    return alignment + sig.second.fullName();
  };

  lines.push_back(alignment + "environment:" + name_);
  lines.push_back(alignment + "P");
  rs::transform(
      parameters_, rs::back_inserter(lines), [&](auto parameter) {
        return alignment + parameter.first + ":" +
               parameter.second.valueAsString();
      });
  lines.push_back(alignment + "I");
  rs::transform(io_.inputs_, rs::back_inserter(lines), pad_signal);
  lines.push_back(alignment + "O");
  rs::transform(io_.outputs_, rs::back_inserter(lines), pad_signal);
  lines.push_back(alignment + "a");
  rs::transform(tags_.pre_, rs::back_inserter(lines), pad_signal);
  lines.push_back(alignment + "b");
  rs::transform(tags_.post_, rs::back_inserter(lines), pad_signal);
  if (with_traces)
  {
    // needs to go
    lines.push_back(alignment + "r");
    rs::transform(
        traces_.pre_, rs::back_inserter(lines), [&](auto trace) {
          return alignment + trace.signal_.fullName() + ";" +
                 std::to_string(trace.frequency_);
        });
    lines.push_back(alignment + "R");
    rs::transform(
        traces_.post_, rs::back_inserter(lines), [&](auto trace) {
          return alignment + trace.signal_.fullName() + ";" +
                 std::to_string(trace.frequency_);
        });
    // needs to go *
  }
  lines.push_back(alignment + "n");
  for (auto nested : nested_)
  {
    lines.push_back(alignment + nested.first);
    auto n_dump = nested.second.e->dump(depth + 1, with_traces);
    lines.insert(lines.end(), n_dump.begin(), n_dump.end());
  }

  return lines;
}

EnvironmentSpec
    EnvironmentSpec::parse(std::vector<std::string> pop_dump)
{
  name_ = *pop_dump.begin();
  name_ = name_.substr(name_.find(':') + 1);

  auto f = rs::begin(pop_dump) + 2;

  for (; *f != "I"; f++)
  {
    auto                   l = *f;
    auto                   p = l.find(':');
    ConfigurationPrimitive c;
    c.parse(l.substr(p + 1));
    parameters_[l.substr(0, p)] = c;
  }

  for (++f; *f != "O"; f++)
  {
    auto l = *f;
    auto p = l.find(':');
    io_.inputs_.push_back({ l.substr(0, p), SignalSpec{ l } });
  }

  for (++f; *f != "a"; f++)
  {
    auto l = *f;
    auto p = l.find(':');
    io_.outputs_.push_back({ l.substr(0, p), SignalSpec{ l } });
  }

  for (++f; *f != "b"; f++)
  {
    auto l = *f;
    auto p = l.find(':');
    tags_.pre_.push_back({ l.substr(0, p), SignalSpec{ l } });
  }

  for (++f; *f != "r"; f++)
  {
    auto l = *f;
    auto p = l.find(':');
    tags_.post_.push_back({ l.substr(0, p), SignalSpec{ l } });
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

    EnvironmentSpec e;
    e.setUserSpecifiedName(*f);
    nested_[*f].e = std::make_unique<EnvironmentSpec>(
        e.parse(std::vector<std::string>(f + 1, p)));

    f = p;
  }
  return *this;
}

std::string
    EnvironmentSpec::prettyPrint()
{
  std::stringstream out;
  out << "environment::" << name_ << "\n{\n";

  out << " parameters\n";
  for (auto [parameter, value] : parameters_)
    out << std::setw(16) << parameter << " : " << value.valueAsString()
        << "\n";
  out << " inputs\n";
  for (auto [input, value] : io_.inputs_)
    out << std::setw(16) << input << " : " << value.fullName() << "\n";
  out << " outputs\n";
  for (auto [output, value] : io_.outputs_)
    out << std::setw(16) << output << " : " << value.fullName() << "\n";
  out << " pre-tags\n";
  for (auto [pre_tag, value] : tags_.pre_)
    out << std::setw(16) << pre_tag << " : " << value.fullName() << "\n";
  out << " post_tag\n";
  for (auto [post_tag, value] : tags_.post_)
    out << std::setw(16) << post_tag << " : " << value.fullName() << "\n";

  out << " nested\n";
  for (auto &[name, nspec] : nested_)
  {
    out << "  " << name << " :\n";
    if (nspec.constraints_.pre_.empty())
      out << "   No pre-constraints\n";
    else
    {
      out << "   pre-constraints:\n";
      for (auto [pre_tag, value] : nspec.constraints_.pre_)
        out << std::setw(16) << pre_tag << " : " << value.fullName() << "\n";
    }
    if (nspec.constraints_.post_.empty())
      out << "   No post-constraints\n";
    else
    {
      out << "   post-constraints:\n";
      for (auto [post_tag, value] : nspec.constraints_.post_)
        out << std::setw(16) << post_tag << " : " << value.fullName() << "\n";
    }
  }

  if (!tag_flow_equalities_.empty() || !tag_flow_inequalities_.empty())
  {
    out << "   with tag-flow-constraints:\n";
    for (auto name : tag_flow_equalities_)
      out << std::setw(20) << name.first.second << "(" << name.first.first
          << ") <=> " << name.second.second << "(" << name.second.first
          << ")\n";
    for (auto name : tag_flow_inequalities_)
      out << std::setw(20) << name.first.second << "(" << name.first.first
          << ") <!=> " << name.second.second << "(" << name.second.first
          << ")\n";
  }
  out << "}\n";
  return out.str();
}

}   // namespace specs
}   // namespace ded
