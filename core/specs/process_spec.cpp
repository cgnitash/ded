
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
  value = rs::find(tags_.pre_, name, &ded::specs::NamedSignal::name_)
              ->signal_spec_.identifier();
}

void
    ProcessSpec::bindPostTag(std::string name, std::string value)
{
  tags_.post_.push_back({ name, SignalSpec{ name, name, value } });
}

void
    ProcessSpec::configurePostTag(std::string name, std::string &value)
{
  value = rs::find(tags_.post_, name, &ded::specs::NamedSignal::name_)
              ->signal_spec_.identifier();
}

void
    ProcessSpec::bindInput(std::string name, std::string value)
{
  io_.inputs_.push_back({ name, SignalSpec{ name, name, value } });
}

void
    ProcessSpec::configureInput(std::string name, ConversionSequence &input)
{
  for (auto &conversion : input_conversions_)
    if (conversion.source_ == name)
      input.push_back(conversion);
}

void
    ProcessSpec::bindOutput(std::string name, std::string value)
{
  io_.outputs_.push_back({ name, SignalSpec{ name, name, value } });
}

void
    ProcessSpec::configureOutput(std::string name, ConversionSequence &output)
{
  for (auto &conversion : output_conversions_)
    if (conversion.sink_ == name)
      output.push_back(conversion);
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
        std::vector<SignalConstraint> pre_constraints)
{
  if (nested_.find(proc_name) == nested_.end())
  {
    std::cout << "User error: nested process vector " << proc_name
              << " has not been declared; cannot bind pre-constraints\n";
    throw SpecError{};
  }
  rs::transform(pre_constraints,
                rs::back_inserter(nested_[proc_name].constraints_.pre_),
				toSignal);
}

void
    ProcessSpec::bindProcessPostConstraints(
        std::string                                      proc_name,
        std::vector<SignalConstraint> post_constraints)
{
  if (nested_.find(proc_name) == nested_.end())
  {
    std::cout << "User error: nested process vector " << proc_name
              << " has not been declared; cannot bind post-constraints\n";
    throw SpecError{};
  }
  rs::transform(post_constraints,
                rs::back_inserter(nested_[proc_name].constraints_.post_),
				toSignal);
}

void
    ProcessSpec::bindProcessVectorPreConstraints(
        std::string                                      proc_name,
        std::vector<SignalConstraint> pre_constraints)
{
  if (nested_vector_.find(proc_name) == nested_vector_.end())
  {
    std::cout << "User error: nested process vector " << proc_name
              << " has not been declared; cannot bind vector pre-constraints\n";
    throw SpecError{};
  }
  rs::transform(pre_constraints,
                rs::back_inserter(nested_vector_[proc_name].second.pre_),
				toSignal);
}

void
    ProcessSpec::bindProcessVectorPostConstraints(
        std::string                                      proc_name,
        std::vector<SignalConstraint> post_constraints)
{
  if (nested_vector_.find(proc_name) == nested_vector_.end())
  {
    std::cout
        << "User error: nested process vector " << proc_name
        << " has not been declared; cannot bind vector post-constraints\n";
    throw SpecError{};
  }
  rs::transform(post_constraints,
                rs::back_inserter(nested_vector_[proc_name].second.post_),
				toSignal);
}

void
    ProcessSpec::bindTagEquality(NamedTag x,
                                 NamedTag y)
{
  if (nested_.find(x.name_) == nested_.end() ||
      nested_.find(y.name_) == nested_.end())
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
    auto &src     = n_tag.signal_spec_;
    auto  matches = rs::count_if(sink_tags_copy, [sig = src](auto ns) {
      return ns.signal_spec_.exactlyMatches(sig);
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
      return ns.signal_spec_.exactlyMatches(sig);
    });

    if (snk->signal_spec_.identifier()[0] == '~')
    {
      if (src.identifier()[0] != '~')
        src.updateIdentifier(snk->signal_spec_.identifier());
      if (snk->signal_spec_.identifier() != src.identifier())
      {
        std::cout
            << "error: tags were previously bound, and are in conflict now\n";
        throw SpecError{};
      }
    }
    else if (src.identifier()[0] == '~')
    {
      if (snk->signal_spec_.identifier()[0] != '~')
        snk->signal_spec_.updateIdentifier(src.identifier());
      if (snk->signal_spec_.identifier() != src.identifier())
      {
        std::cout << "error: tags were previously bound, and are in "
                     "conflict now\n";
        throw SpecError{};
      }
    }
    else
    {
      tag_count++;
      snk->signal_spec_.updateIdentifier("~tag" + std::to_string(tag_count));
      src.updateIdentifier("~tag" + std::to_string(tag_count));
    }

    sink_tags_copy.erase(rs::find_if(sink_tags_copy, [sig = src](auto ns) {
      return ns.signal_spec_.exactlyMatches(sig);
    }));
  }
}

void
    ProcessSpec::matchTagFlowEqualities(int &tag_count)
{

  for (auto [source, sink] : tag_flow_equalities_)
  {
    auto &source_tags = source.type_ == TagType::pre
                            ? nested_[source.name_].e->tags_.pre_
                            : nested_[source.name_].e->tags_.post_;
    auto &sink_tags = sink.type_ == TagType::pre
                          ? nested_[sink.name_].e->tags_.pre_
                          : nested_[sink.name_].e->tags_.post_;
    matchTags(source_tags, sink_tags, tag_count);
  }
}

void
    ProcessSpec::updateNestedConstraints(SignalSpecSet &constraints)
{
  for (auto &source_tag : constraints)
  {
    auto same_user_name = [&source_tag](auto tag) {
      return tag.signal_spec_.userName() == source_tag.signal_spec_.userName();
    };
    if (auto f = rs::find_if(tags_.pre_, same_user_name);
        f != rs::end(tags_.pre_))
      source_tag.signal_spec_.updateIdentifier(f->signal_spec_.identifier());
    else if (f = rs::find_if(tags_.post_, same_user_name);
             f != rs::end(tags_.post_))
      source_tag.signal_spec_.updateIdentifier(f->signal_spec_.identifier());
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
    for (auto &[param, cp] : parameters_.parameters_)
      if (cp.typeAsString() == "long" && param == n_sig.signal_spec_.userParameter())
        n_sig.signal_spec_.instantiateUserParameter(std::stol(cp.valueAsString()));

  for (auto &n_sig : io_.outputs_)
    for (auto &[param, cp] : parameters_.parameters_)
      if (cp.typeAsString() == "long" && param == n_sig.signal_spec_.userParameter())
        n_sig.signal_spec_.instantiateUserParameter(std::stol(cp.valueAsString()));

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
  for (auto &oc : input_conversion_sequence_)
    bindInputSignal(oc, ios.inputs_, sub_spec.nameToken());
  for (auto &oc : output_conversion_sequence_)
    bindOutputSignal(oc, ios.outputs_, sub_spec.nameToken());
}

void
    ProcessSpec::convertSignalConversionSequence(
        language::Block::TokenBlocks converter,
        SignalSpec                   &sig,
        language::Token              &source_token,
        ConversionSequence_          &cs)
{
	auto name = converter.name_;
	auto block = converter.blocks_[0];

    if (!config_manager::isConverterBlock(name.expr_.substr(1)))
    {
      errInvalidToken(name, "Not a converter component");
      throw language::ParserError{};
    }

	//auto converter_spec = ALL_CONVERTER_SPECS[name.expr_];
	auto converter_spec = ConverterSpec{block};

    auto [in, out] = converter_spec.args();

    if (!sig.convertibleTo(in))
    {
      errInvalidToken(source_token,
                      " output_type cannot be converted : output_type = " +
                          sig.diagnosticName());
      errInvalidToken(name,
                      " to expected input_type : input_type = " +
                          in.diagnosticName());
      throw language::ParserError{};
    }

    cs.sequence_.push_back(converter_spec.signature());
    cs.specs_.push_back(converter_spec);
	//might not be needed
	//cs.names_.push_back(converter_spec.name());

    in.updatePlaceholders(sig);
    out.updatePlaceholders(in);
    sig = out;
	source_token = name;
}

void
    ProcessSpec::bindInputSignal(
        language::Block::TokenBlockSignalBind signal_conversion_sequence,
        SignalSpecSet                         sigs,
        language::Token                       sub_spec_name)
{

	/*
  auto sequence = signal_conversion_sequence.expr_;
  sequence.erase(rs::remove(sequence, ' '), rs::end(sequence));
  auto expression = sequence.substr(1, sequence.size() - 2);

  auto lst = expression | rv::split('>') | rs::to<std::vector<std::string>>;

  if (lst.size() < 2)
  {
    errInvalidToken(signal_conversion_sequence,
                    "unrecognized conversion sequence");
    throw language::ParserError{};
  }
	*/
  ConversionSequence_ cs;

  auto source_token = signal_conversion_sequence.source_;
  auto source = rs::find(io_.inputs_, source_token.expr_, &NamedSignal::name_);
  if (source == rs::end(io_.inputs_))
  {
    errInvalidToken(source_token, "Not an input signal provided by " + name_);
    throw language::ParserError{};
  }
  cs.source_ = source_token.expr_;

  auto sink_token = signal_conversion_sequence.sink_;
  auto sink       = rs::find(sigs, sink_token.expr_, &NamedSignal::name_);
  if (sink == rs::end(sigs))
  {
    errInvalidToken(sink_token,
                        "Not an input signal provided by " +
                        sub_spec_name.expr_);
    throw language::ParserError{};
  }
  cs.sink_ = sink_token.expr_;

  auto sig = source->signal_spec_;

  //for (auto i = 1u; i < lst.size() - 1; i++)
  for (auto converter : signal_conversion_sequence.sequence_)
  {
	  convertSignalConversionSequence(converter, sig, source_token, cs);
  }

  if (!sig.convertibleTo(sink->signal_spec_))
  {
    errInvalidToken(source_token,
                    " output_type cannot be converted : output_type = " +
                        sig.diagnosticName());
    errInvalidToken(sink_token,
                    " to expected input_type : input_type = " +
                        sink->signal_spec_.diagnosticName());
    throw language::ParserError{};
  }

  input_conversions_.push_back(cs);
}

void ProcessSpec::bindOutputSignal(
    language::Block::TokenBlockSignalBind signal_conversion_sequence,
    SignalSpecSet sigs,
    language::Token sub_spec_name)
{

	/*
  auto sequence = signal_conversion_sequence.expr_;
  sequence.erase(rs::remove(sequence, ' '), rs::end(sequence));
  auto expression = sequence.substr(1, sequence.size() - 2);

  auto lst = expression | rv::split('>') | rs::to<std::vector<std::string>>;

  if (lst.size() < 2)
  {
    errInvalidToken(signal_conversion_sequence,
                    "unrecognized conversion sequence");
    throw language::ParserError{};
  }
*/

  ConversionSequence_ cs;

  auto source_token = signal_conversion_sequence.source_;
  auto source = rs::find(sigs, source_token.expr_, &NamedSignal::name_);
  if (source == rs::end(sigs))
  {
    errInvalidToken(source_token, "Not an output signal provided by " + sub_spec_name.expr_);
    throw language::ParserError{};
  }
  cs.source_ = source_token.expr_;

  auto sink_token = signal_conversion_sequence.sink_;
  auto sink       = rs::find(io_.outputs_, sink_token.expr_, &NamedSignal::name_);
  if (sink == rs::end(io_.outputs_))
  {
    errInvalidToken(sink_token, "Not an output signal provided by " + name_);
    throw language::ParserError{};
  }
  cs.sink_ = sink_token.expr_;

  auto sig = source->signal_spec_;

  for (auto converter : signal_conversion_sequence.sequence_)
  {
	  convertSignalConversionSequence(converter, sig, source_token, cs);
  }
  /*
  for (auto i = 1u; i < lst.size() - 1; i++)
  {
    if (!config_manager::isConverterBlock(lst[i]))
    {
      errInvalidToken(signal_conversion_sequence,
                      lst[i] + " is not a converter component");
      throw language::ParserError{};
    }
	   
	auto converter_spec = 	ALL_CONVERTER_SPECS[lst[i]];

    auto [in, out] = converter_spec.args();

    if (!sig.convertibleTo(in))
    {
      errInvalidToken(signal_conversion_sequence,
                      lst[i - 1] + " result cannot be converted to input of " +
                          lst[i]);
      throw language::ParserError{};
    }

	cs.sequence_.push_back(converter_spec.signature());
	cs.names_.push_back(converter_spec.name());

    in.updatePlaceholders(sig);
    out.updatePlaceholders(in);
    sig = out;
  }
	*/
  if (!sig.convertibleTo(sink->signal_spec_))
  {
    errInvalidToken(source_token,
                    " output_type cannot be converted : output_type = " +
                        sig.diagnosticName());
    errInvalidToken(sink_token,
                    " to expected input_type : input_type = " +
                        sink->signal_spec_.diagnosticName());
    throw language::ParserError{};
  }

  output_conversions_.push_back(cs);
}

/*
void
    ProcessSpec::errSignalBind(SignalSpec      proc_sig,
                               language::Token sub_spec_name,
                               SignalSpecSet   sub_sigs,
                               bool            is_input)
{
  auto diagnostic_message = name_token_.diagnostic_;
  auto left_padding       = std::string(name_token_.location_.column_ + 10, ' ');
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
      std::string(substrate_name_token.location_.column_ + 10, ' ');

  std::cout << ss_diagnostic_message << "\n"
            << ss_left_padding << utilities::TermColours::red_fg << "^"
            << std::string(substrate_name_token.expr_.length() - 1, '~') << "\n"
            << ss_left_padding << "viable " << (is_input ? "input" : "output")
            << " signal candidates provided\n"
            << utilities::TermColours::reset;

  for (auto sig : sub_sigs)
    std::cout << ss_left_padding << sig.signal_spec_.diagnosticName() << std::endl;
  throw SpecError{};
}

bool
    ProcessSpec::attemptExplicitBind(SignalSpec &    proc_sig,
                                     SignalSpecSet   sub_sigs,
                                     language::Token sub_spec_name,
                                     bool is_input)
{

  auto sig_bind = rs::find_if(signal_binds_, [&proc_sig](auto token_pair) {
    return token_pair.lhs_.expr_ == proc_sig.userName();
  });
  if (sig_bind == rs::end(signal_binds_))
    return false;

  auto conversion_sequence = specs::parseConversionSequence( sig_bind->rhs_.expr_);

  auto sub_sig = rs::find_if(sub_sigs, [sig_bind](auto ns) {
    return ns.name_ == sig_bind->rhs_.expr_;
  });

  if (sub_sig == rs::end(sub_sigs))
  {
      std::string io_type = is_input ? "input" : "output";
      errInvalidToken(sig_bind->rhs_,
                      "this is not an " + io_type + " signal provided by " +
                          sub_spec_name.expr_,
                      sub_sigs | rv::transform(&NamedSignal::name_) |
                          rs::to<std::vector<std::string>>);
      throw language::ParserError{};
  }

  if (!sub_sig->signal_spec_.exactlyMatches(proc_sig))
  {
	errSignalBind(proc_sig, sub_spec_name, sub_sigs, is_input); 
    throw language::ParserError{};
  }

  proc_sig.updateIdentifier(sub_sig->signal_spec_.identifier());
  return true;
}

void
    ProcessSpec::bindSignalsTo(SignalSpecSet   sub_sigs,
                               language::Token sub_spec_name,
                               bool            is_input)
{
  auto &proc_sigs = is_input ? io_.inputs_ : io_.outputs_;
  for (auto &proc_name_sig : proc_sigs)
  {
    auto &proc_sig = proc_name_sig.signal_spec_;
    if (!attemptExplicitBind(proc_sig, sub_sigs, sub_spec_name, is_input))
    {
      std::string io_type = is_input ? "input" : "output";
      errInvalidToken(name_token_,
                      io_type + " signal " + proc_name_sig.name_ +
                          " is not bound to an " + io_type + " signal of " +
                          sub_spec_name.expr_);
      throw language::ParserError{};
    }

	// BELOW SHOULD BE REMOVED
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
*/
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
        rs::find(tags_.pre_, sig_freq.signal_.userName(), &NamedSignal::name_)
            ->signal_spec_.identifier());

  for (auto &sig_freq : traces_.post_)
    sig_freq.signal_.updateIdentifier(
        rs::find(tags_.post_, sig_freq.signal_.userName(), &NamedSignal::name_)
            ->signal_spec_.identifier());

  for (auto &es : nested_)
    es.second.e->recordTraces();

  for (auto &esvec : nested_vector_)
    for (auto &es : esvec.second.first)
      es.e->recordTraces();
}

void
    ProcessSpec::parseTraces(language::Block block)
{
  for (auto over : block.traces_)
  {
    auto tag_name  = over.lhs_;
    auto frequency = over.rhs_;

    ConfigurationPrimitive cp;
    cp.parse(frequency.expr_);
    if (cp.typeAsString() != "long")
    {
      errInvalidToken(frequency, "expected frequency of trace (number) here");
      throw language::ParserError{};
    }

    if (auto i = rs::find(tags_.pre_, tag_name.expr_, &NamedSignal::name_);
        i != rs::end(tags_.pre_))
    {
      traces_.pre_.push_back(
          { i->signal_spec_.fullName(), std::stoi(frequency.expr_) });
    }
    else if (i = rs::find(tags_.post_, tag_name.expr_, &NamedSignal::name_);
             i != rs::end(tags_.post_))
    {
      traces_.post_.push_back(
          { i->signal_spec_.fullName(), std::stoi(frequency.expr_) });
    }
    else
    {
      errInvalidToken(
          tag_name,
          "this is not a tag recognised by " + name_,
          rv::concat(tags_.pre_ | rv::transform(&NamedSignal::name_),
                     tags_.post_ | rv::transform(&NamedSignal::name_)) |
              rs::to<std::vector<std::string>>);

      throw language::ParserError{};
    }
  }
}

void
    ProcessSpec::parseNested(language::Block block)
{
  for (auto blover : block.nested_)
  {
    auto name         = blover.name_;
    auto nested_block = blover.blocks_[0];

    auto f = rs::find_if(nested_,
                         [&](auto param) { return param.first == name.expr_; });
    if (f == nested_.end())
    {
      errInvalidToken(name,
                      "this does not override any nested processs " +
                          block.name_,
                      nested_ | rv::keys | rs::to<std::vector<std::string>>);
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
    auto name = blover.name_;

    auto f = rs::find_if(nested_vector_,
                         [&](auto param) { return param.first == name.expr_; });
    if (f == nested_vector_.end())
    {
      errInvalidToken(name,
                      "this does not override any nested vector of processes " +
                          block.name_,
                      nested_vector_ | rv::keys |
                          rs::to<std::vector<std::string>>);
      throw language::ParserError{};
    }

    for (auto [i, nested_block] : rv::enumerate(blover.blocks_))
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
	input_conversion_sequence_ = block.input_signal_binds_;
	output_conversion_sequence_ = block.output_signal_binds_;
	/*
  signal_binds_ = block.signal_binds_;

  for (auto signal_bind : signal_binds_)
  {
    auto doesnt_contain_signal = [](auto sigs, auto sig_bind) {
      return rs::find_if(sigs, [&](auto sig) {
               return sig_bind.lhs_.expr_ == sig.name_;
             }) == rs::end(sigs);
    };
    if (doesnt_contain_signal(io_.inputs_, signal_bind) &&
        doesnt_contain_signal(io_.outputs_, signal_bind))
    {
      errInvalidToken(signal_bind.lhs_,
                      "this is not an io signal of " + name_);
      throw language::ParserError{};
    }
  }
   */
}

ProcessSpec::ProcessSpec(language::Block block)
{
  auto block_name = block.name_.substr(1);
  if (!rs::contains(config_manager::allProcessNames(), block_name))
  {
    errInvalidToken(block.name_token_,
                    "this is not an exisiting Process-component",
                    config_manager::allProcessNames());
    throw SpecError{};
  }

  *this = ALL_PROCESS_SPECS.at(block_name);

  name_token_ = block.name_token_;

  parameters_.loadFromSpec(block.overrides_, name_);
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

  auto pad_signal = [&](auto sig) { return alignment + sig.signal_spec_.fullName(); };

  lines.push_back(alignment + "process:" + name_);
  lines.push_back(alignment + "PARAMETERS");
  rs::transform(parameters_.parameters_, rs::back_inserter(lines), [&](auto parameter) {
    return alignment + parameter.first + ":" + parameter.second.valueAsString();
  });
  lines.push_back(alignment + "INPUTS");
  for (auto &conversion : input_conversions_)
  {
    lines.push_back(alignment + conversion.source_);
    lines.push_back(alignment + conversion.sink_);
    for (auto &conv : conversion.specs_)
    {
      lines.push_back(alignment + "-");
      auto c_dump = conv.serialise(depth);
      lines.insert(lines.end(), c_dump.begin(), c_dump.end());
      lines.push_back(alignment + "-");
    }
  }
  lines.push_back(alignment + "OUTPUTS");
  for (auto &conversion : output_conversions_)
  {
    lines.push_back(alignment + conversion.source_);
    lines.push_back(alignment + conversion.sink_);
    for (auto &conv : conversion.specs_)
    {
      lines.push_back(alignment + "-");
      auto c_dump = conv.serialise(depth);
      lines.insert(lines.end(), c_dump.begin(), c_dump.end());
      lines.push_back(alignment + "-");
    }
  }
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
    parameters_.parameters_[l.substr(0, p)] = c;
  }

  for (++f; *f != "OUTPUTS";)
  {
    ConversionSequence_ cs;
    cs.source_ = *f++;
    cs.sink_   = *f++;

    while (*f == "-")
    {
      auto p = std::find_if(
          f + 1, pop_dump.end(), [](auto l) { return l[0] == '-'; });

      ConverterSpec e;
      e.deserialise(std::vector<std::string>(f + 1, p));
      cs.specs_.push_back(e);

      f = p + 1;
    }
    input_conversions_.push_back(cs);
  }

  //TEST
  /*
  std::cout <<  "testing input_conversions_ \n";
  for (auto sequence : input_conversions_)
  { 
	  std::cout <<  sequence[0] << "\n";
	  for (auto i = 1u; i < sequence.size(); i++)
            std::cout << " " << sequence[i] << "\n";
  }
  */

  for (++f; *f != "PRETAGS";)
  {
    ConversionSequence_ cs;
    cs.source_ = *f++;
    cs.sink_   = *f++;

    while (*f == "-")
    {
      auto p = std::find_if(
          f + 1, pop_dump.end(), [](auto l) { return l[0] == '-'; });

      ConverterSpec e;
      e.deserialise(std::vector<std::string>(f + 1, p));
      cs.specs_.push_back(e);

      f = p + 1;
    }
    output_conversions_.push_back(cs);
  }

  //TEST
  /*
  std::cout <<  "testing output_conversions_ \n";
  for (auto sequence : output_conversions_)
  { 
	  std::cout <<  sequence[0] << "\n";
	  for (auto i = 1u; i < sequence.size(); i++)
            std::cout << " " << sequence[i] << "\n";
  }
  */

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
    out << nspec.e->prettyPrint();
  }

  if (!tag_flow_equalities_.empty() || !tag_flow_inequalities_.empty())
  {
    out << "   with tag-flow-constraints:\n";
    for (auto tag_flow : tag_flow_equalities_)
      out << std::setw(20) << tag_flow.from_.type_ << "(" << tag_flow.from_.name_
          << ") <=> " << tag_flow.to_.type_ << "(" << tag_flow.to_.name_
          << ")\n";
    for (auto tag_flow : tag_flow_inequalities_)
      out << std::setw(20) << tag_flow.from_.type_ << "(" << tag_flow.from_.name_
          << ") <!=> " << tag_flow.to_.type_ << "(" << tag_flow.to_.name_
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
        out << std::setw(20) << name.name_ << ":" << name.signal_spec_.type() << "\n";
    }
    if (!tags.post_.empty())
    {
      out << "   with post-tag-constraints:\n";
      for (auto name : tags.post_)
        out << std::setw(20) << name.name_ << ":" << name.signal_spec_.type() << "\n";
    }
    for (auto &nspec : nspec_vec_tags.first)
      out << nspec.e->prettyPrint();
  }
  return out.str();
}

std::string
    ProcessSpec::prettyPrint()
{
  std::stringstream out;
  out << "process::" << name_ << "\n{\n";

  out << " parameters\n";
  for (auto [parameter, value] : parameters_.parameters_)
    out << std::setw(16) << parameter << " : " << value.valueAsString() << "\n";
  out << " inputs\n";
  for (auto [input, value] : io_.inputs_)
    out << std::setw(16) << input << " : " << value.type() << "\n";
  out << " outputs\n";
  for (auto [output, value] : io_.outputs_)
    out << std::setw(16) << output << " : " << value.type() << "\n";
  /*
  out << " input_conversions\n";
  for (auto seq : input_conversions_)
  	for (auto s : seq)
    	out << std::setw(16) << s << "\n";
  out << " output_conversions_\n";
  for (auto seq : output_conversions_)
  	for (auto s : seq)
    	out << std::setw(16) << s << "\n";
		*/
  out << " pre-tags\n";
  for (auto [pre_tag, value] : tags_.pre_)
    out << std::setw(16) << pre_tag << " : " << value.type() << "\n";
  out << " post_tag\n";
  for (auto [post_tag, value] : tags_.post_)
    out << std::setw(16) << post_tag << " : " << value.type() << "\n";

  out << " nested\n" << prettyPrintNested();
  //out << " nested-vector\n" << prettyPrintNestedVector();
  out << "}\n";
  return out.str();
}

}   // namespace specs
}   // namespace ded
