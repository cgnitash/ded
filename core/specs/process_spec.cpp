
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <string>

#include "../configuration.hpp"
#include "process_spec.hpp"

namespace ded
{
namespace specs
{

void
    ProcessSpec::bindPreTag(std::string name, std::string value)
{
  tags_.pre_.push_back({ name, SignalSpec{ name, name, value } });
}

void
    ProcessSpec::configurePreTag(std::string name, std::string &value)
{
  value = rs::find_if(tags_.pre_, [=](auto ns) { return ns.first == name; })
              ->second.identifier();
}

void
    ProcessSpec::bindPostTag(std::string name, std::string value)
{
  tags_.post_.push_back({ name, SignalSpec{ name, name, value } });
}

void
    ProcessSpec::configurePostTag(std::string name, std::string &value)
{
  value = rs::find_if(tags_.post_, [=](auto ns) { return ns.first == name; })
              ->second.identifier();
}

void
    ProcessSpec::bindInput(std::string name, std::string value)
{
  io_.inputs_.push_back({ name, SignalSpec{ name, name, value } });
}

void
    ProcessSpec::configureInput(std::string name, std::string &value)
{
  value = rs::find_if(io_.inputs_, [=](auto ns) { return ns.first == name; })
              ->second.identifier();
}

void
    ProcessSpec::bindOutput(std::string name, std::string value)
{
  io_.outputs_.push_back({ name, SignalSpec{ name, name, value } });
}

void
    ProcessSpec::configureOutput(std::string name, std::string &value)
{
  value = rs::find_if(io_.outputs_, [=](auto ns) { return ns.first == name; })
              ->second.identifier();
}
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
  /*
  nested_[proc_name].constraints_.pre_ =
      pre_constraints |
      rv::transform([](auto tag) -> std::pair<std::string, SignalSpec> {
        auto name  = tag.first;
        auto value = tag.second;
        return { name, SignalSpec{ name, name, value } };
      });
	  */
  rs::transform(pre_constraints,
                rs::back_inserter(nested_[proc_name].constraints_.pre_),
                [](auto tag) -> std::pair<std::string, SignalSpec> {
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
  /*
  nested_[proc_name].constraints_.post_ =
      post_constraints |
      rv::transform([](auto tag) -> std::pair<std::string, SignalSpec> {
        auto name  = tag.first;
        auto value = tag.second;
        return { name, SignalSpec{ name, name, value } };
      });
	  */
  rs::transform(post_constraints,
                rs::back_inserter(nested_[proc_name].constraints_.post_),
                [](auto tag) -> std::pair<std::string, SignalSpec> {
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
  /*
  nested_vector_[proc_name].second.pre_ =
      pre_constraints |
      rv::transform([](auto tag) -> std::pair<std::string, SignalSpec> {
        auto name  = tag.first;
        auto value = tag.second;
        return { name, SignalSpec{ name, name, value } };
      });
  */
  rs::transform(pre_constraints,
                rs::back_inserter(nested_vector_[proc_name].second.pre_),
                [](auto tag) -> std::pair<std::string, SignalSpec> {
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
  /*
  nested_vector_[proc_name].second.post_ =
      post_constraints |
      rv::transform([](auto tag) -> std::pair<std::string, SignalSpec> {
        auto name  = tag.first;
        auto value = tag.second;
        return { name, SignalSpec{ name, name, value } };
      });
  */
  rs::transform(post_constraints,
                rs::back_inserter(nested_vector_[proc_name].second.post_),
                [](auto tag) -> std::pair<std::string, SignalSpec> {
                  auto name  = tag.first;
                  auto value = tag.second;
                  return { name, SignalSpec{ name, name, value } };
                });
}

void
    ProcessSpec::bindTagEquality(std::pair<std::string, std::string> x,
                                 std::pair<std::string, std::string> y)
{
  auto is_pre_post = [](auto s) { return s == "pre" || s == "post"; };

  if (nested_.find(x.first) == nested_.end() ||
      nested_.find(y.first) == nested_.end() || !is_pre_post(x.second) ||
      !is_pre_post(y.second))
  {
    std::cout << "User error: cannot bind tag equality \n";
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

  auto ios = sub_spec.getIO();
  bindSignalsTo(ios.inputs_, sub_spec.nameToken(), true);
  bindSignalsTo(ios.outputs_, sub_spec.nameToken(), false);
}

void
    ProcessSpec::errSignalBind(SignalSpec      proc_sig,
                               language::Token sub_spec_name,
                               SignalSpecSet   sub_sigs,
                               bool            is_input)
{
  auto diagnostic_message = name_token_.diagnostic_;
  auto left_padding       = std::string(name_token_.location_.second + 10, ' ');
  std::cout << "parse-error\n"
            << diagnostic_message << "\n"
            << left_padding << utilities::TermColours::red_fg << "^"
            << std::string(name_token_.expr_.length() - 1, '~') << "\n"
            << left_padding << (is_input ? "input" : "output")
            << " signals provided by substrate cannot be bound\n"
            << utilities::TermColours::reset << left_padding
            << proc_sig.diagnosticName() << "\n\n";

  auto substrate_name_token  = sub_spec_name;
  auto ss_diagnostic_message = substrate_name_token.diagnostic_;
  auto ss_left_padding =
      std::string(substrate_name_token.location_.second + 10, ' ');

  std::cout << ss_diagnostic_message << "\n"
            << ss_left_padding << utilities::TermColours::red_fg << "^"
            << std::string(substrate_name_token.expr_.length() - 1, '~') << "\n"
            << ss_left_padding << "viable " << (is_input ? "input" : "output")
            << " signal candidates provided\n"
            << utilities::TermColours::reset;

  for (auto sig : sub_sigs)
    std::cout << ss_left_padding << sig.second.diagnosticName() << std::endl;
  throw SpecError{};
}

bool
    ProcessSpec::attemptExplicitBind(SignalSpec &    proc_sig,
                                     SignalSpecSet   sub_sigs,
                                     language::Token sub_spec_name,
                                     bool /*is_input*/)
{

  auto sig_bind = rs::find_if(signal_binds_, [&proc_sig](auto token_pair) {
    return token_pair.first.expr_ == proc_sig.userName();
  });
  if (sig_bind == rs::end(signal_binds_))
    return false;

  auto sub_sig = rs::find_if(sub_sigs, [sig_bind](auto ns) {
    return ns.first == sig_bind->second.expr_;
  });

  if (sub_sig == rs::end(sub_sigs))
  {
    errInvalidToken(sig_bind->second,
                    "not a signal provided by " + sub_spec_name.expr_);
    throw language::ParserError{};
  }

  if (!sub_sig->second.exactlyMatches(proc_sig))
  {
    errInvalidToken(sig_bind->first, "signal required by " + name_);
    errInvalidToken(sig_bind->second,
                    "cannot be bound by " + sub_spec_name.expr_);
    throw language::ParserError{};
  }

  std::cout << sub_sig->second.identifier() << " # " << proc_sig.identifier();
  proc_sig.updateIdentifier(sub_sig->second.identifier());
  return true;
}

void
    ProcessSpec::bindSignalsTo(SignalSpecSet   sub_sigs,
                               language::Token sub_spec_name,
                               bool            is_input)
{
  auto proc_sigs = is_input ? io_.inputs_ : io_.outputs_;
  for (auto &proc_name_sig : proc_sigs)
  {
    auto &proc_sig = proc_name_sig.second;
    if (attemptExplicitBind(proc_sig, sub_sigs, sub_spec_name, is_input))
      continue;
    auto matches = rs::count_if(sub_sigs, [proc_sig](auto ns) {
      return ns.second.exactlyMatches(proc_sig);
    });
    if (matches != 1)
    {
      errSignalBind(proc_sig, sub_spec_name, sub_sigs, is_input);
    }
    auto i = rs::find_if(sub_sigs, [proc_sig](auto ns) {
      return ns.second.exactlyMatches(proc_sig);
    });
    proc_sig.updateIdentifier(i->second.identifier());
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
          parameters_ | rv::keys | rs::to<std::vector<std::string>>);
          //parameters_ | rv::transform([](auto param) { return param.first; }));
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
              //tags_.pre_ |  rv::transform([](auto tag) { return tag.first; }),
          tags_.pre_ | rv::keys,// | rs::to<std::vector<std::string>>,
              //tags_.post_ | rv::transform([](auto tag) { return tag.first; })));
          tags_.post_ | rv::keys) | rs::to<std::vector<std::string>>);

      throw language::ParserError{};
    }
  }
}

void
    ProcessSpec::parseNested(language::Block block)
{
  for (auto blover : block.nested_)
  {
    auto name         = blover.first;
    auto nested_block = blover.second;

    auto f = rs::find_if(nested_,
                         [&](auto param) { return param.first == name.expr_; });
    if (f == nested_.end())
    {
      errInvalidToken(
          name,
          "this does not override any nested processs " + block.name_,
          nested_ | rv::keys | rs::to<std::vector<std::string>>);
		  //rv::transform([](auto param) { return param.first; }));
      throw language::ParserError{};
    }

	/*
    if (config_manager::typeOfBlock(nested_block.name_.substr(1)) !=
        config_manager::SpecType::process)
    {
      errInvalidToken(name,
                      "override of " + name.expr_ + " must be of type process",
                      config_manager::allProcessNames());
      throw language::ParserError{};
    }
	*/

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
          nested_vector_ | rv::keys | rs::to<std::vector<std::string>>);
                  //    nested_vector_ | rv::transform([](auto param) {
                  //      return param.first;
                  //    }));
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
      ns.e = std::make_unique<ProcessSpec>(ProcessSpec{ nested_block });
      ns.constraints_ = f->second.second;
      ns.e->setUserSpecifiedName(name.expr_ + "_" + std::to_string(i));
      f->second.first.push_back(ns);
    }
  }
}

void
    ProcessSpec::parseSignalBinds(language::Block block)
{
  signal_binds_ = block.signal_binds_;

  for (auto signal_bind : signal_binds_)
  {
    auto doesnt_contain_signal = [](auto sigs, auto sig_bind) {
      return rs::find_if(sigs, [&](auto sig) {
               return sig_bind.first.expr_ == sig.first;
             }) == rs::end(sigs);
    };
    if (doesnt_contain_signal(io_.inputs_, signal_bind) &&
        doesnt_contain_signal(io_.outputs_, signal_bind))
    {
      errInvalidToken(signal_bind.first,
                      "this is not an io signal of " + name_);
      throw language::ParserError{};
    }
  }
}

ProcessSpec::ProcessSpec(language::Block block)
{
  auto block_name = block.name_.substr(1);
  if (rs::none_of(config_manager::allProcessNames(),
                  [&](auto comp_name) { return comp_name == block_name; }))
  {
    errInvalidToken(block.name_token_,
                    "this is not an exisiting Process-component",
                    config_manager::allProcessNames());
    throw SpecError{};
  }

  *this = ALL_PROCESS_SPECS.at(block_name);

  name_token_ = block.name_token_;

  parseParameters(block);
  parseSignalBinds(block);
  parseTraces(block);
  parseNested(block);
  parseNestedVector(block);
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
    ProcessSpec::prettyPrintNested()
{
  std::stringstream out;
  for (auto &[name, nspec] : nested_)
  {
    out << "  " << name << " :\n";
    if (nspec.constraints_.pre_.empty())
      out << "   No pre-constraints\n";
    else
    {
      out << "   pre-constraints:\n";
      for (auto [pre_tag, value] : nspec.constraints_.pre_)
        out << std::setw(16) << pre_tag << " : " << value.type() << "\n";
    }
    if (nspec.constraints_.post_.empty())
      out << "   No post-constraints\n";
    else
    {
      out << "   post-constraints:\n";
      for (auto [post_tag, value] : nspec.constraints_.post_)
        out << std::setw(16) << post_tag << " : " << value.type() << "\n";
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
  return out.str();
}

std::string
    ProcessSpec::prettyPrintNestedVector()
{
  std::stringstream out;
  for (auto &[name, nspec_vec_tags] : nested_vector_)
  {
    out << "  " << name << " :\n";
    for (auto &nspec : nspec_vec_tags.first)
    {
      if (nspec.constraints_.pre_.empty())
        out << "   No pre-constraints\n";
      else
      {
        out << "   pre-constraints:\n";
        for (auto [pre_tag, value] : nspec.constraints_.pre_)
          out << std::setw(16) << pre_tag << " : " << value.type() << "\n";
      }
      if (nspec.constraints_.post_.empty())
        out << "   No post-constraints\n";
      else
      {
        out << "   post-constraints:\n";
        for (auto [post_tag, value] : nspec.constraints_.post_)
          out << std::setw(16) << post_tag << " : " << value.type() << "\n";
      }
    }

    auto &tags = nspec_vec_tags.second;

    if (!tags.pre_.empty())
    {
      out << "   with pre-tag-constraints:\n";
      for (auto name : tags.pre_)
        out << std::setw(20) << name.first << ":" << name.second.type() << "\n";
    }
    if (!tags.post_.empty())
    {
      out << "   with post-tag-constraints:\n";
      for (auto name : tags.post_)
        out << std::setw(20) << name.first << ":" << name.second.type() << "\n";
    }
  }
  return out.str();
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
    out << std::setw(16) << input << " : " << value.type() << "\n";
  out << " outputs\n";
  for (auto [output, value] : io_.outputs_)
    out << std::setw(16) << output << " : " << value.type() << "\n";
  out << " pre-tags\n";
  for (auto [pre_tag, value] : tags_.pre_)
    out << std::setw(16) << pre_tag << " : " << value.type() << "\n";
  out << " post_tag\n";
  for (auto [post_tag, value] : tags_.post_)
    out << std::setw(16) << post_tag << " : " << value.type() << "\n";

  out << " nested\n" << prettyPrintNested();
  out << " nested-vector\n" << prettyPrintNestedVector();
  out << "}\n";
  return out.str();
}

}   // namespace specs
}   // namespace ded
