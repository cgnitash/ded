
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

void ProcessSpec::checkInvalidTokens(language::Block)
{
}

void
    ProcessSpec::preTag(std::string name, std::string type)
{
  if (!isConfigurable)
    tags_.pre_.push_back({ name, SignalSpec{ type } });
}

void
    ProcessSpec::postTag(std::string name, std::string type)
{
  if (!isConfigurable)
    tags_.post_.push_back({ name, SignalSpec{ type } });
}

void
    ProcessSpec::input(std::string                  name,
                       std::string                  value,
                       ConversionSignatureSequence &input)
{

  if (isConfigurable)
    configureInput(name, input);
  else
    bindInput(name, value);
}

void
    ProcessSpec::bindInput(std::string name, std::string type)
{
  io_.inputs_.push_back({ name, SignalSpec{ type } });
}

void
    ProcessSpec::configureInput(std::string                  name,
                                ConversionSignatureSequence &input)
{
  input = input_conversions_ |
          rv::filter([=](auto c) { return c.source_ == name; }) |
          rv::transform(makeConversionSignatureSequence) |
          rs::to<ConversionSignatureSequence>;
}

void
    ProcessSpec::output(std::string                  name,
                        std::string                  value,
                        ConversionSignatureSequence &output)
{

  if (isConfigurable)
    configureOutput(name, output);
  else
    bindOutput(name, value);
}

void
    ProcessSpec::bindOutput(std::string name, std::string type)
{
  io_.outputs_.push_back({ name, SignalSpec{ type } });
}

void
    ProcessSpec::configureOutput(std::string                  name,
                                 ConversionSignatureSequence &output)
{
  output = output_conversions_ |
           rv::filter([=](auto c) { return c.sink_ == name; }) |
           rv::transform(makeConversionSignatureSequence) |
           rs::to<ConversionSignatureSequence>;
}

void
    ProcessSpec::nestedProcess(std::string                   name,
                               ProcessSpec &                 proc,
                               std::vector<SignalConstraint> pre_constraints,
                               std::vector<SignalConstraint> post_constraints)
{
  if (isConfigurable)
    configureProcess(name, proc);
  else
  {
    bindProcess(name, proc);
    bindProcessPreConstraints(name, pre_constraints);
    bindProcessPostConstraints(name, post_constraints);
  }
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
    ProcessSpec::nestedProcessVector(
        std::string                   name,
        std::vector<ProcessSpec> &    procs,
        std::vector<SignalConstraint> pre_constraints,
        std::vector<SignalConstraint> post_constraints)
{
  if (isConfigurable)
    configureProcessVector(name, procs);
  else
  {
    bindProcessVector(name, procs);
    bindProcessVectorPreConstraints(name, pre_constraints);
    bindProcessVectorPostConstraints(name, post_constraints);
  }
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
        std::string                   proc_name,
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
        std::string                   proc_name,
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
        std::string                   proc_name,
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
        std::string                   proc_name,
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
    ProcessSpec::bindTags()
{
  for (auto &es : nested_)
    es.second.e->bindTags();

  for (auto &c : tag_conversion_sequence_)
    bindTagConversionSequence(c);

  for (auto n : nested_)
  {
    for (auto tag : n.second.e->tags_.pre_)
      if (!tag.signal_spec_.isBound())
      {
        errInvalidToken(name_token_,
                        "pre tag " + tag.name_ +
                            " is not bound to a post tag of " + n.first);
        throw language::ParserError{};
      }
    for (auto tag : n.second.e->tags_.post_)
      if (!tag.signal_spec_.isBound())
      {
        errInvalidToken(name_token_,
                        "post tag " + tag.name_ +
                            " is not bound to a pre tag of " + n.first);
        throw language::ParserError{};
      }
  }

  for (auto nv : nested_vector_)
    for (auto [i, n] : rv::enumerate(nv.second.first))
    {
      for (auto tag : n.e->tags_.pre_)
        if (!tag.signal_spec_.isBound())
        {
          errInvalidToken(name_token_,
                          "pre tag " + tag.name_ + " of " + nv.first + ":" +
                              std::to_string(i) +
                              " is not bound to a post tag");
          throw language::ParserError{};
        }
      for (auto tag : n.e->tags_.post_)
        if (!tag.signal_spec_.isBound())
        {
          errInvalidToken(name_token_,
                          "post tag " + tag.name_ + " of " + nv.first + ":" +
                              std::to_string(i) + " is not bound to a pre tag");
          throw language::ParserError{};
        }
    }
}

std::pair<NamedSignal, std::string>
    ProcessSpec::getTagsWithName(language::Token token, bool is_pre)
{
  if (token.type_ == language::TokenType::tag_name)
  {
    auto p           = token.expr_.find(':');
    auto nested_name = token.expr_.substr(0, p);
    auto nested_tag  = token.expr_.substr(p + 1);

    if (auto p2 = nested_tag.find(':'); p2 != std::string::npos)
    {
      auto nested_vector_id =
          std::stol(nested_tag.substr(0, p2));   // check this is numeric
      nested_tag = nested_tag.substr(p2 + 1);

      if (nested_vector_.find(nested_name) == nested_vector_.end())
      {
        errInvalidToken(
            token,
            "<" + nested_name + "> is not a nested process of " + name_,
            nested_vector_ | rv::keys | rs::to<std::vector<std::string>>);
        throw language::ParserError{};
      }

      auto &ret_tags_nested = is_pre ? nested_vector_[nested_name]
                                           .first[nested_vector_id]
                                           .e->tags_.post_
                                     : nested_vector_[nested_name]
                                           .first[nested_vector_id]
                                           .e->tags_.pre_;

      auto ret_tag_itr_nested =
          rs::find(ret_tags_nested, nested_tag, &NamedSignal::name_);
      auto &ret_tags_constraints = is_pre ? nested_vector_[nested_name]
                                                .first[nested_vector_id]
                                                .constraints_.pre_
                                          : nested_vector_[nested_name]
                                                .first[nested_vector_id]
                                                .constraints_.post_;
      auto  ret_tag_itr_constraints =
          rs::find(ret_tags_constraints, nested_tag, &NamedSignal::name_);
      if (ret_tag_itr_nested == rs::end(ret_tags_nested) &&
          ret_tag_itr_constraints == rs::end(ret_tags_constraints))
      {
        errInvalidToken(token,
                        nested_tag + " is not a valid tag ",
                        rv::concat(ret_tags_nested, ret_tags_constraints) |
                            rv::transform(&NamedSignal::name_) |
                            rs::to<std::vector<std::string>>);
        throw language::ParserError{};
      }
      auto &ret_tag_itr = ret_tag_itr_nested == rs::end(ret_tags_nested)
                              ? ret_tag_itr_constraints
                              : ret_tag_itr_nested;
      ret_tag_itr->signal_spec_.setBound();
      return { *ret_tag_itr, nested_tag };
    }
    else
    {
      if (nested_.find(nested_name) == nested_.end())
      {
        errInvalidToken(token,
                        "<" + nested_name + "> is not a nested process of " +
                            name_,
                        nested_ | rv::keys | rs::to<std::vector<std::string>>);
        throw language::ParserError{};
      }
      auto &ret_tags_nested = is_pre ? nested_[nested_name].e->tags_.post_
                                     : nested_[nested_name].e->tags_.pre_;
      auto  ret_tag_itr_nested =
          rs::find(ret_tags_nested, nested_tag, &NamedSignal::name_);
      auto &ret_tags_constraints =
          is_pre ? nested_[nested_name].constraints_.pre_
                 : nested_[nested_name].constraints_.post_;
      auto ret_tag_itr_constraints =
          rs::find(ret_tags_constraints, nested_tag, &NamedSignal::name_);
      if (ret_tag_itr_nested == rs::end(ret_tags_nested) &&
          ret_tag_itr_constraints == rs::end(ret_tags_constraints))
      {
        errInvalidToken(token,
                        nested_tag + " is not a valid tag ",
                        rv::concat(ret_tags_nested, ret_tags_constraints) |
                            rv::transform(&NamedSignal::name_) |
                            rs::to<std::vector<std::string>>);
        throw language::ParserError{};
      }
      auto &ret_tag_itr = ret_tag_itr_nested == rs::end(ret_tags_nested)
                              ? ret_tag_itr_constraints
                              : ret_tag_itr_nested;
      ret_tag_itr->signal_spec_.setBound();
      return { *ret_tag_itr, nested_tag };
    }
  }
  else   // this case might be completely unnecessary
  {
    auto &ret_tags    = is_pre ? tags_.pre_ : tags_.post_;
    auto  ret_tag_itr = rs::find(ret_tags, token.expr_, &NamedSignal::name_);
    if (ret_tag_itr == rs::end(ret_tags))
    {
      errInvalidToken(token,
                      token.expr_ + " is not a post tag ",
                      ret_tags | rv::transform(&NamedSignal::name_) |
                          rs::to<std::vector<std::string>>);
      throw language::ParserError{};
    }
    ret_tag_itr->signal_spec_.setBound();
    return { *ret_tag_itr, token.expr_ };
  }
}

void
    ProcessSpec::bindTagConversionSequence(
        language::Block::TokenBlockSignalBind tag_conversion_sequence)
{
  ConversionSequence_ cs;

  auto source_token              = tag_conversion_sequence.source_;
  auto [source_sig, source_name] = getTagsWithName(source_token, true);
  cs.source_                     = source_name;

  auto sink_token            = tag_conversion_sequence.sink_;
  auto [sink_sig, sink_name] = getTagsWithName(sink_token, false);
  cs.sink_                   = sink_name;

  auto sig = source_sig.signal_spec_;

  for (auto converter : tag_conversion_sequence.sequence_)
    convertSignalConversionSequence(converter, sig, source_token, cs);

  if (!sig.convertibleTo(sink_sig.signal_spec_))
  {
    errInvalidToken(source_token,
                    " result type of this converter is : " +
                        sig.diagnosticName());
    errInvalidToken(sink_token,
                    " which cannot be converted to expected type : " +
                        sink_sig.signal_spec_.diagnosticName());
    throw language::ParserError{};
  }

  if (source_token.type_ == language::TokenType::tag_name)
  {
    auto p           = source_token.expr_.find(':');
    auto nested_name = source_token.expr_.substr(0, p);
    auto nested_tag  = source_token.expr_.substr(p + 1);
    if (auto p2 = nested_tag.find(':'); p2 != std::string::npos)
    {
      auto nested_vector_id = std::stol(nested_tag.substr(0, p2));
      nested_vector_[nested_name]
          .first[nested_vector_id]
          .e->tag_conversions_.push_back(cs);
    }
    else
    {
      nested_[nested_name].e->tag_conversions_.push_back(cs);
    }
  }
  else   // may not be needed at all
  {
    tag_conversions_.push_back(cs);
  }
}

void
    ProcessSpec::instantiateUserParameterSizes()
{
  for (auto &n_sig : io_.inputs_)
    n_sig.signal_spec_.instantiateUserParameter(parameters_.parameters_);

  for (auto &n_sig : io_.outputs_)
    n_sig.signal_spec_.instantiateUserParameter(parameters_.parameters_);

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

  for (auto &oc : input_conversion_sequence_)
    bindSignalConversionSequence(oc, sub_spec, true);
  for (auto &oc : output_conversion_sequence_)
    bindSignalConversionSequence(oc, sub_spec, false);

  for (auto spec : io_.outputs_)
    if (!spec.signal_spec_.isBound())
    {
      errInvalidToken(name_token_,
                      "output signal <" + spec.name_ +
                          "> must be completely bound");
      throw language::ParserError{};
    }
  auto ios = sub_spec.getIO();
  for (auto spec : ios.inputs_)
    if (spec.signal_spec_.isPartiallyBounded())
    {
      std::string s;
      for (int i : spec.signal_spec_.unboundIndices())
        s += std::to_string(i) + " ";
      errInvalidToken(name_token_,
                      "input signals " + spec.name_ +
                          " cannot be partially provided to "
                          "substrates: unbound_indices [ " +
                          s + "]");
      throw language::ParserError{};
    }

  // only warnings
  for (auto spec : io_.inputs_)
    if (spec.signal_spec_.isPartiallyBounded())
    {
      std::string s;
      for (int i : spec.signal_spec_.unboundIndices())
        s += std::to_string(i) + " ";
      errWarningToken(
          name_token_,
          "this process generates an input signal \n    " + spec.name_ +
              " :  " + spec.signal_spec_.diagnosticName() +
              " \nAre you sure you want to partially read from this "
              "input? \nUnread indices [ " +
              s + "]");
      // throw language::ParserError{};
    }
  for (auto spec : ios.outputs_)
    if (spec.signal_spec_.isPartiallyBounded())
    {
      std::string s;
      for (int i : spec.signal_spec_.unboundIndices())
        s += std::to_string(i) + " ";
      errWarningToken(name_token_,
                      "this process is reading an output signal of \n  " +
                          sub_spec.name() + "\n    " + spec.name_ + " :  " +
                          spec.signal_spec_.diagnosticName() +
                          " \nAre you sure you want to partially read from "
                          "this substrate's output? \nUnread indices [ " +
                          s + "]");
      // throw language::ParserError{};
    }
}

void
    ProcessSpec::bindSignalConversionSequence(
        language::Block::TokenBlockSignalBind signal_conversion_sequence,
        specs::SubstrateSpec &                sub_spec,
        bool                                  is_input)
{

  ConversionSequence_ cs;

  auto message = is_input ? std::string{ "input" } : std::string{ "output" };

  auto &sub_ios     = sub_spec.getIO();
  auto &source_sigs = is_input ? io_.inputs_ : sub_ios.outputs_;
  auto &sink_sigs   = is_input ? sub_ios.inputs_ : io_.outputs_;

  auto source_token = signal_conversion_sequence.source_;
  auto source = rs::find(source_sigs, source_token.expr_, &NamedSignal::name_);
  if (source == rs::end(source_sigs))
  {
    errInvalidToken(source_token,
                    "Not an " + message + " signal of " +
                        (is_input ? name_ : sub_spec.name()),
                    source_sigs | rv::transform(&NamedSignal::name_) |
                        rs::to<std::vector<std::string>>);
    throw language::ParserError{};
  }
  cs.source_ = source_token.expr_;

  auto sink_token = signal_conversion_sequence.sink_;
  auto sink       = rs::find(sink_sigs, sink_token.expr_, &NamedSignal::name_);
  if (sink == rs::end(sink_sigs))
  {
    errInvalidToken(sink_token,
                    "Not an " + message + " signal of " +
                        (is_input ? sub_spec.name() : name_),
                    sink_sigs | rv::transform(&NamedSignal::name_) |
                        rs::to<std::vector<std::string>>);
    throw language::ParserError{};
  }
  cs.sink_ = sink_token.expr_;

  auto sig = source->signal_spec_;

  for (auto converter : signal_conversion_sequence.sequence_)
    convertSignalConversionSequence(converter, sig, source_token, cs);

  if (!sig.convertibleTo(sink->signal_spec_))
  {
    errInvalidToken(source_token,
                    " result type of this converter is\n    " +
                        sig.diagnosticName());
    errInvalidToken(sink_token,
                    " which cannot be converted to expected type\n    " +
                        sink->signal_spec_.diagnosticName());
    throw language::ParserError{};
  }

  if (source->signal_spec_.isVectorType())
  {
    // and first conversion is not SLICE
    // source->signal_spec_.setBound();
    if (!cs.specs_.empty() and cs.specs_.front().name() == "slice")
    {
      auto [from, to, every, vtt] = cs.specs_.front().getSliceRange();
      source->signal_spec_.addBoundIndices(
          rv::iota(from, to) | rv::stride(every) | rs::to<std::vector<int>>);
    }
    else
      source->signal_spec_.setBound();
  }
  else
    source->signal_spec_.setBound();

  if (sink->signal_spec_.isVectorType())
  {
    // and last conversion was not APPLY
    sink->signal_spec_.setBound();
  }
  else
    sink->signal_spec_.setBound();

  if (is_input)
    input_conversions_.push_back(cs);
  else
    output_conversions_.push_back(cs);
}

void
    ProcessSpec::convertSignalConversionSequence(
        language::Block::TokenBlocks converter,
        SignalSpec &                 sig,
        language::Token &            source_token,
        ConversionSequence_ &        cs)
{
  auto name  = converter.name_;
  auto block = converter.blocks_[0];

  if (!config_manager::isConverterBlock(block.name_token_.expr_.substr(1)))
  {
    errInvalidToken(
        name, "Not a converter component", config_manager::allConverterNames());
    throw language::ParserError{};
  }

  auto converter_spec = ConverterSpec{ block };

  auto [in, out] = converter_spec.args();

  if (block.name_token_.expr_ == "$slice")
  {
    auto [from, to, every, vtt] = converter_spec.getSliceRange();
    if (!sig.sliceableBy(from, to, every, vtt))
    {
      errInvalidToken(source_token,
                      " result type of this converter is : " +
                          sig.diagnosticName());
      errInvalidToken(name,
                      " which cannot be sliced to indices [" +
                          std::to_string(from) + ":" + std::to_string(to) +
                          "," + std::to_string(every) + "]");
      throw language::ParserError{};
    }
    converter_spec.forceParameter("vtt", vtt);
  }
  else if (!sig.convertibleTo(in))
  {
    errInvalidToken(source_token,
                    " result type of this converter is\n    " +
                        sig.diagnosticName());
    errInvalidToken(name,
                    " which cannot be converted to expected type\n    " +
                        in.diagnosticName());
    throw language::ParserError{};
  }

  cs.specs_.push_back(converter_spec);

  in.updatePlaceholders(sig);
  out.updatePlaceholders(in);
  sig          = out;
  source_token = name;
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
      traces_.pre_.push_back({ i->name_, std::stoi(frequency.expr_) });
    }
    else if (i = rs::find(tags_.post_, tag_name.expr_, &NamedSignal::name_);
             i != rs::end(tags_.post_))
    {
      traces_.post_.push_back({ i->name_, std::stoi(frequency.expr_) });
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
      if (!config_manager::isProcessBlock(nested_block.name_.substr(1)))
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
  input_conversion_sequence_  = block.input_signal_binds_;
  output_conversion_sequence_ = block.output_signal_binds_;
}

void
    ProcessSpec::parseTagBinds(language::Block block)
{
  tag_conversion_sequence_ = block.tag_binds_;
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

  checkInvalidTokens(block);

  name_token_ = block.name_token_;

  parameters_.loadFromSpec(block.overrides_, name_);
  parseSignalBinds(block);
  parseTraces(block);
  parseNested(block);
  parseNestedVector(block);
  parseTagBinds(block);
}

std::vector<std::string>
    ProcessSpec::serialise(long depth, bool with_traces) const
{
  std::vector<std::string> lines;
  auto                     alignment = std::string(depth, ' ');

  lines.push_back(alignment + "process:" + name_);
  lines.push_back(alignment + "PARAMETERS");
  rs::transform(
      parameters_.parameters_, rs::back_inserter(lines), [&](auto parameter) {
        return alignment + parameter.first + ":" +
               parameter.second.valueAsString();
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
  lines.push_back(alignment + "TAGS");
  for (auto &conversion : tag_conversions_)
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
  if (with_traces)
  {
    // needs to go
    lines.push_back(alignment + "PRETRACES");
    rs::transform(traces_.pre_, rs::back_inserter(lines), [&](auto trace) {
      return alignment + trace.name_ + ";" + std::to_string(trace.frequency_);
    });
    lines.push_back(alignment + "POSTTRACES");
    rs::transform(traces_.post_, rs::back_inserter(lines), [&](auto trace) {
      return alignment + trace.name_ + ";" + std::to_string(trace.frequency_);
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

  for (++f; *f != "TAGS";)
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

  for (++f; *f != "PRETRACES";)
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
    tag_conversions_.push_back(cs);
  }

  for (++f; *f != "POSTTRACES"; f++)
  {
    auto l = *f;
    auto p = l.find(';');
    traces_.pre_.push_back({ l.substr(0, p), std::stoi(l.substr(p + 1)) });
  }

  for (++f; *f != "NESTED"; f++)
  {
    auto l = *f;
    auto p = l.find(';');
    traces_.post_.push_back({ l.substr(0, p), std::stoi(l.substr(p + 1)) });
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
        out << std::setw(20) << name.name_ << ":" << name.signal_spec_.type()
            << "\n";
    }
    if (!tags.post_.empty())
    {
      out << "   with post-tag-constraints:\n";
      for (auto name : tags.post_)
        out << std::setw(20) << name.name_ << ":" << name.signal_spec_.type()
            << "\n";
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
