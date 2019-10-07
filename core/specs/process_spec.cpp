
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <string>

#include "../configuration.h"
#include "process_spec.h"

namespace ded
{
namespace specs
{

void
    ProcessSpec::bindProcess(std::string name, ProcessSpec proc)
{
  if (nested_.find(name) != nested_.end())
  {
    std::cout << "User error: nested process " << name
              << " has already been declared\n";
    throw SpecError{};
  }
  nested_[name].e = std::make_unique<ProcessSpec>(proc);
}

void
    ProcessSpec::configureProcess(std::string name, ProcessSpec &proc)
{
  if (nested_.find(name) == nested_.end())
  {
    std::cout << "User error: nested process " << name
              << " has not been declared\n";
    throw SpecError{};
  }
  proc = *nested_[name].e;
}

void
    ProcessSpec::bindProcessVector(std::string              name,
                                   std::vector<ProcessSpec> procs)
{
  if (nested_vector_.find(name) != nested_vector_.end())
  {
    std::cout << "User error: nested process vector " << name
              << " has already been declared\n";
    throw SpecError{};
  }
  nested_vector_[name];
  for (auto proc : procs)
  {
    NestedProcessSpec ns;
    ns.e = std::make_unique<ProcessSpec>(proc);
    nested_vector_[name].first.push_back(ns);
  }
}

void
    ProcessSpec::configureProcessVector(std::string               name,
                                        std::vector<ProcessSpec> &procs)
{
  if (nested_vector_.find(name) == nested_vector_.end())
  {
    std::cout << "User error: nested process vector " << name
              << " has not been declared\n";
    throw SpecError{};
  }
  procs.clear();
  for (auto ns : nested_vector_[name].first)
  {
    auto np = *ns.e;
    procs.push_back(np);
  }
}

void
    ProcessSpec::bindProcessPreConstraints(
        std::string                                      proc_name,
        std::vector<std::pair<std::string, std::string>> pre_constraints)
{
  if (nested_.find(proc_name) == nested_.end())
  {
    std::cout << "User error: nested process vector " << proc_name
              << " has not been declared; cannot bind pre-constraints\n";
    throw SpecError{};
  }
  nested_[proc_name].constraints_.pre_ =
      pre_constraints |
      rv::transform([](auto tag) -> std::pair<std::string, SignalSpec> {
        auto name  = tag.first;
        auto value = tag.second;
        return { name, SignalSpec{ name, name, value } };
      });
}

void
    ProcessSpec::bindProcessPostConstraints(
        std::string                                      proc_name,
        std::vector<std::pair<std::string, std::string>> post_constraints)
{
  if (nested_.find(proc_name) == nested_.end())
  {
    std::cout << "User error: nested process vector " << proc_name
              << " has not been declared; cannot bind post-constraints\n";
    throw SpecError{};
  }
  nested_[proc_name].constraints_.post_ =
      post_constraints |
      rv::transform([](auto tag) -> std::pair<std::string, SignalSpec> {
        auto name  = tag.first;
        auto value = tag.second;
        return { name, SignalSpec{ name, name, value } };
      });
}

void
    ProcessSpec::bindProcessVectorPreConstraints(
        std::string                                      proc_name,
        std::vector<std::pair<std::string, std::string>> pre_constraints)
{
  // tags_.post_.push_back({ name, SignalSpec{ name, name, value } });
  if (nested_vector_.find(proc_name) == nested_vector_.end())
  {
    std::cout << "User error: nested process vector " << proc_name
              << " has not been declared; cannot bind vector pre-constraints\n";
    throw SpecError{};
  }
  nested_vector_[proc_name].second.pre_ =
      pre_constraints |
      rv::transform([](auto tag) -> std::pair<std::string, SignalSpec> {
        auto name  = tag.first;
        auto value = tag.second;
        return { name, SignalSpec{ name, name, value } };
      });
}

void
    ProcessSpec::bindProcessVectorPostConstraints(
        std::string                                      proc_name,
        std::vector<std::pair<std::string, std::string>> post_constraints)
{
  if (nested_vector_.find(proc_name) == nested_vector_.end())
  {
    std::cout
        << "User error: nested process vector " << proc_name
        << " has not been declared; cannot bind vector post-constraints\n";
    throw SpecError{};
  }
  nested_vector_[proc_name].second.post_ =
      post_constraints |
      rv::transform([](auto tag) -> std::pair<std::string, SignalSpec> {
        auto name  = tag.first;
        auto value = tag.second;
        return { name, SignalSpec{ name, name, value } };
      });
}

void
    ProcessSpec::bindTagEquality(std::pair<std::string, std::string> x,
                                 std::pair<std::string, std::string> y)
{
  auto        is_pre_post   = [](auto s) { return s == "pre" || s == "post"; };
  std::string error_message = "User error: cannot bind tag equality ";
  if (nested_.find(x.first) == nested_.end() ||
      nested_.find(y.first) == nested_.end() || !is_pre_post(x.second) ||
      !is_pre_post(y.second))
  {
    std::cout << "User error: " << error_message << "\n";
    throw SpecError{};
  }

  tag_flow_equalities_.push_back({ x, y });
}

void
    ProcessSpec::matchTags(SignalSpecSet &source_tags,
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
    ProcessSpec::matchTagFlowEqualities(int &tag_count)
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
    ProcessSpec::updateNestedConstraints(SignalSpecSet &constraints)
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
    ProcessSpec::updateAndMatchTags(SignalSpecSet &source_tags,
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
    ProcessSpec::matchNestedTagConstraints(int &tag_count)
{
  for (auto &ename_es : nested_)
  {
    auto &es = ename_es.second;
    updateAndMatchTags(es.constraints_.pre_, es.e->tags_.pre_, tag_count);
    updateAndMatchTags(es.constraints_.post_, es.e->tags_.post_, tag_count);
  }

  for (auto &es_vec : nested_vector_)
    for (auto &es : es_vec.second.first)
    {
      updateAndMatchTags(es.constraints_.pre_, es.e->tags_.pre_, tag_count);
      updateAndMatchTags(es.constraints_.post_, es.e->tags_.post_, tag_count);
    }
}

void
    ProcessSpec::bindTags(int tag_count)
{

  matchTagFlowEqualities(tag_count);

  matchNestedTagConstraints(tag_count);

  for (auto &es : nested_)
    es.second.e->bindTags(tag_count);
}

void
    ProcessSpec::instantiateUserParameterSizes()
{
  for (auto &n_sig : io_.inputs_)
    for (auto &[param, cp] : parameters_)
      if (cp.typeAsString() == "long" && param == n_sig.second.userParameter())
        n_sig.second.instantiateUserParameter(std::stol(cp.valueAsString()));

  for (auto &n_sig : io_.outputs_)
    for (auto &[param, cp] : parameters_)
      if (cp.typeAsString() == "long" && param == n_sig.second.userParameter())
        n_sig.second.instantiateUserParameter(std::stol(cp.valueAsString()));

  for (auto &es : nested_)
    es.second.e->instantiateUserParameterSizes();

  for (auto &esvec : nested_vector_)
    for (auto &es : esvec.second.first)
      es.e->instantiateUserParameterSizes();
}

void
    ProcessSpec::bindSubstrateIO(SubstrateSpec sub_spec)
{
  for (auto &es : nested_)
    es.second.e->bindSubstrateIO(sub_spec);

  for (auto &esvec : nested_vector_)
    for (auto &es : esvec.second.first)
      es.e->bindSubstrateIO(sub_spec);

  bindSignalTo(sub_spec);
}

void
    ProcessSpec::errSignalBind(SubstrateSpec sub_spec,
                               SignalSpec    sig,
                               bool          is_input)
{
  auto diagnostic_message = name_token_.diagnostic_;
  auto left_padding       = std::string(name_token_.location_.second + 10, ' ');
  std::cout << "parse-error\n\n"
            << diagnostic_message << "\n"
            << left_padding << utilities::TermColours::red_fg << "^"
            << std::string(name_token_.expr_.length() - 1, '~') << "\n"
            << left_padding << "no " << (is_input ? "input" : "output")
            << " signals provided by substrate can be bound\n"
            << utilities::TermColours::reset << left_padding
            << sig.diagnosticName() << "\n\n";

  auto substrate_name_token  = sub_spec.nameToken();
  auto ss_diagnostic_message = substrate_name_token.diagnostic_;
  auto ss_left_padding =
      std::string(substrate_name_token.location_.second + 10, ' ');

  std::cout << ss_diagnostic_message << "\n"
            << ss_left_padding << utilities::TermColours::red_fg << "^"
            << std::string(substrate_name_token.expr_.length() - 1, '~') << "\n"
            << ss_left_padding << "viable " << (is_input ? "input" : "output")
            << " signal candidates provided\n"
            << utilities::TermColours::reset;

  auto valid_signals =
      is_input ? sub_spec.getIO().inputs_ : sub_spec.getIO().outputs_;
  for (auto sig : valid_signals)
    std::cout << ss_left_padding << sig.second.diagnosticName() << std::endl;
  throw SpecError{};
}

void
    ProcessSpec::bindSignalTo(SubstrateSpec sub_spec)
{
  auto ios = sub_spec.getIO();

  for (auto &n_sig : io_.inputs_)
  {
    auto &in_sig  = n_sig.second;
    auto  matches = rs::count_if(ios.inputs_, [sig = in_sig](auto ns) {
      return ns.second.exactlyMatches(sig);
    });
    if (matches > 1)
    {
      std::cout << "error: multiple input signals match exactly\n";
      throw SpecError{};
    }
    if (!matches)
    {
      errSignalBind(sub_spec, n_sig.second, true);
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
      throw SpecError{};
    }
    if (!matches)
    {
      errSignalBind(sub_spec, n_sig.second, false);
    }
    auto i = rs::find_if(ios.outputs_, [sig = out_sig](auto ns) {
      return ns.second.exactlyMatches(sig);
    });
    out_sig.updateIdentifier(i->second.identifier());
    ios.outputs_.erase(i);
  }
}

std::vector<std::pair<Trace, std::string>>
    ProcessSpec::queryTraces()
{

  std::vector<std::pair<Trace, std::string>> res;

  for (auto &sig_freq : traces_.pre_)
    res.push_back({ sig_freq, {} });

  for (auto &sig_freq : traces_.post_)
    res.push_back({ sig_freq, {} });

  for (auto &es : nested_)
    for (auto ts : es.second.e->queryTraces())
      res.push_back(ts);

  for (auto &esvec : nested_vector_)
    for (auto &es : esvec.second.first)
      for (auto ts : es.e->queryTraces())
        res.push_back(ts);

  for (auto &r : res)
    r.second = user_specified_name_ + "_" + name_ + "/" + r.second;

  return res;
}

void
    ProcessSpec::recordTraces()
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

  for (auto &esvec : nested_vector_)
    for (auto &es : esvec.second.first)
      es.e->recordTraces();
}

void
    ProcessSpec::parseParameters(language::Block block)
{
  for (auto over : block.overrides_)
  {
    auto name  = over.first;
    auto value = over.second;

    auto f = rs::find_if(parameters_,
                         [&](auto param) { return param.first == name.expr_; });
    if (f == parameters_.end())
    {
      errInvalidToken(
          name,
          "this does not override any parameters of " + name_,
          parameters_ | rv::transform([](auto param) { return param.first; }));
      throw language::ParserError{};
    }

    ConfigurationPrimitive cp;
    cp.parse(value.expr_);
    if (cp.typeAsString() != f->second.typeAsString())
    {
      using namespace std::literals::string_literals;
      errInvalidToken(value,
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
      errInvalidToken(value, "parameter constraint not satisfied: " + *con);
      throw language::ParserError{};
    }
  }
}

void
    ProcessSpec::parseTraces(language::Block block)
{
  for (auto over : block.traces_)
  {
    auto tag_name  = over.first;
    auto frequency = over.second;

    ConfigurationPrimitive cp;
    cp.parse(frequency.expr_);
    if (cp.typeAsString() != "long")
    {
      errInvalidToken(frequency, "expected frequency of trace (number) here");
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
      errInvalidToken(
          tag_name,
          "this is not a tag recognised by " + name_,
          rv::concat(
              tags_.pre_ | rv::transform([](auto tag) { return tag.first; }),
              tags_.post_ | rv::transform([](auto tag) { return tag.first; })));
      throw language::ParserError{};
    }
  }
}

void
    ProcessSpec::parseNested(language::Block block)
{
  for (auto blover : block.nested_)
  {
    auto name       = blover.first;
    auto nested_block = blover.second;

    auto f = rs::find_if(nested_,
                         [&](auto param) { return param.first == name.expr_; });
    if (f == nested_.end())
    {
      errInvalidToken(
          name,
          "this does not override any nested processs " + block.name_,
          nested_ | rv::transform([](auto param) { return param.first; }));
      throw language::ParserError{};
    }

    if (config_manager::typeOfBlock(nested_block.name_.substr(1)) !=
        config_manager::SpecType::process)
    {
      errInvalidToken(name,
                      "override of " + name.expr_ + " must be of type process",
                      config_manager::allProcessNames());
      throw language::ParserError{};
    }

    f->second.e = std::make_unique<ProcessSpec>(ProcessSpec{ nested_block });
    f->second.e->setUserSpecifiedName(name.expr_);
  }
}

void
    ProcessSpec::parseNestedVector(language::Block block)
{
  for (auto blover : block.nested_vector_)
  {
    auto name = blover.first;

    auto f = rs::find_if(nested_vector_,
                         [&](auto param) { return param.first == name.expr_; });
    if (f == nested_vector_.end())
    {
      errInvalidToken(name,
                      "this does not override any nested vector of processes " +
                          block.name_,
                      nested_vector_ | rv::transform([](auto param) {
                        return param.first;
                      }));
      throw language::ParserError{};
    }

    for (auto [i, nested_block] : rv::enumerate(blover.second))
    {
    if (config_manager::typeOfBlock(nested_block.name_.substr(1)) !=
        config_manager::SpecType::process)
      {
        errInvalidToken(name,
                        "nested process vector of " + name.expr_ +
                            " must be of type process",
                        config_manager::allProcessNames());
        throw language::ParserError{};
      }

      NestedProcessSpec ns;
      ns.e = std::make_unique<ProcessSpec>(ProcessSpec{  nested_block });
      ns.constraints_ = f->second.second;
      ns.e->setUserSpecifiedName(name.expr_ + "_" + std::to_string(i));
      f->second.first.push_back(ns);
    }
  }
}

ProcessSpec::ProcessSpec( language::Block block)
{

  *this = ALL_PROCESS_SPECS.at(block.name_.substr(1));

  name_token_ = block.name_token_;

  parseParameters( block);
  parseTraces( block);
  parseNested( block);
  parseNestedVector( block);
}

std::vector<std::string>
    ProcessSpec::serialise(long depth, bool with_traces) const
{
  std::vector<std::string> lines;
  auto                     alignment = std::string(depth, ' ');

  auto pad_signal = [&](auto sig) { return alignment + sig.second.fullName(); };

  lines.push_back(alignment + "process:" + name_);
  lines.push_back(alignment + "PARAMETERS");
  rs::transform(parameters_, rs::back_inserter(lines), [&](auto parameter) {
    return alignment + parameter.first + ":" + parameter.second.valueAsString();
  });
  lines.push_back(alignment + "INPUTS");
  rs::transform(io_.inputs_, rs::back_inserter(lines), pad_signal);
  lines.push_back(alignment + "OUTPUTS");
  rs::transform(io_.outputs_, rs::back_inserter(lines), pad_signal);
  lines.push_back(alignment + "PRETAGS");
  rs::transform(tags_.pre_, rs::back_inserter(lines), pad_signal);
  lines.push_back(alignment + "POSTTAGS");
  rs::transform(tags_.post_, rs::back_inserter(lines), pad_signal);
  if (with_traces)
  {
    // needs to go
    lines.push_back(alignment + "PRETRACES");
    rs::transform(traces_.pre_, rs::back_inserter(lines), [&](auto trace) {
      return alignment + trace.signal_.fullName() + ";" +
             std::to_string(trace.frequency_);
    });
    lines.push_back(alignment + "POSTTRACES");
    rs::transform(traces_.post_, rs::back_inserter(lines), [&](auto trace) {
      return alignment + trace.signal_.fullName() + ";" +
             std::to_string(trace.frequency_);
    });
    // needs to go *
  }
  lines.push_back(alignment + "NESTED");
  for (auto const &nested : nested_)
  {
    lines.push_back(alignment + nested.first);
    auto n_dump = nested.second.e->serialise(depth + 1, with_traces);
    lines.insert(lines.end(), n_dump.begin(), n_dump.end());
  }
  lines.push_back(alignment + "NESTEDVECTOR");
  for (auto const &nested_vector : nested_vector_)
  {
    for (auto const &nested : nested_vector.second.first)
    {
      lines.push_back(alignment + nested_vector.first);
      auto n_dump = nested.e->serialise(depth + 1, with_traces);
      lines.insert(lines.end(), n_dump.begin(), n_dump.end());
    }
  }

  return lines;
}

ProcessSpec
    ProcessSpec::deserialise(std::vector<std::string> pop_dump)
{
  name_ = *pop_dump.begin();
  name_ = name_.substr(name_.find(':') + 1);

  auto f = rs::begin(pop_dump) + 2;

  for (; *f != "INPUTS"; f++)
  {
    auto                   l = *f;
    auto                   p = l.find(':');
    ConfigurationPrimitive c;
    c.parse(l.substr(p + 1));
    parameters_[l.substr(0, p)] = c;
  }

  for (++f; *f != "OUTPUTS"; f++)
  {
    auto l = *f;
    auto p = l.find(':');
    io_.inputs_.push_back({ l.substr(0, p), SignalSpec{ l } });
  }

  for (++f; *f != "PRETAGS"; f++)
  {
    auto l = *f;
    auto p = l.find(':');
    io_.outputs_.push_back({ l.substr(0, p), SignalSpec{ l } });
  }

  for (++f; *f != "POSTTAGS"; f++)
  {
    auto l = *f;
    auto p = l.find(':');
    tags_.pre_.push_back({ l.substr(0, p), SignalSpec{ l } });
  }

  for (++f; *f != "PRETRACES"; f++)
  {
    auto l = *f;
    auto p = l.find(':');
    tags_.post_.push_back({ l.substr(0, p), SignalSpec{ l } });
  }

  for (++f; *f != "POSTTRACES"; f++)
  {
    auto l = *f;
    auto p = l.find(';');
    traces_.pre_.push_back({ l.substr(0, p - 1), std::stoi(l.substr(p + 1)) });
  }

  for (++f; *f != "NESTED"; f++)
  {
    auto l = *f;
    auto p = l.find(';');
    traces_.post_.push_back({ l.substr(0, p - 1), std::stoi(l.substr(p + 1)) });
  }

  for (++f; *f != "NESTEDVECTOR";)
  {
    auto p =
        std::find_if(f + 1, pop_dump.end(), [](auto l) { return l[0] != ' '; });

    std::transform(f + 1, p, f + 1, [](auto l) { return l.substr(1); });

    ProcessSpec e;
    e.setUserSpecifiedName(*f);
    nested_[*f].e = std::make_unique<ProcessSpec>(
        e.deserialise(std::vector<std::string>(f + 1, p)));

    f = p;
  }

  int nv_count = 0;
  for (++f; f != pop_dump.end(); nv_count++)
  {
    auto p =
        std::find_if(f + 1, pop_dump.end(), [](auto l) { return l[0] != ' '; });

    std::transform(f + 1, p, f + 1, [](auto l) { return l.substr(1); });

    ProcessSpec e;
    e.setUserSpecifiedName(*f + "_" + std::to_string(nv_count));
    NestedProcessSpec ns;
    ns.e = std::make_unique<ProcessSpec>(
        e.deserialise(std::vector<std::string>(f + 1, pop_dump.end())));
    nested_vector_[*f].first.push_back(ns);

    f = p;
  }
  return *this;
}

std::string
    ProcessSpec::prettyPrint()
{
  std::stringstream out;
  out << "process::" << name_ << "\n{\n";

  out << " parameters\n";
  for (auto [parameter, value] : parameters_)
    out << std::setw(16) << parameter << " : " << value.valueAsString() << "\n";
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
