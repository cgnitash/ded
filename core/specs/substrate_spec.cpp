
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../configuration.hpp"
#include "../utilities/utilities.hpp"
#include "substrate_spec.hpp"

namespace ded
{
namespace specs
{

void
    SubstrateSpec::bindInput(std::string name, std::string value)
{
  io_.inputs_.push_back({ name, SignalSpec{ name, name, value } });
}

void
    SubstrateSpec::configureInput(std::string name, std::string &value)
{
  auto i = rs::find_if(io_.inputs_, [=](auto ns) { return ns.first == name; });
  value  = i->second.identifier();
}

void
    SubstrateSpec::bindOutput(std::string name, std::string value)
{
  io_.outputs_.push_back({ name, SignalSpec{ name, name, value } });
}

void
    SubstrateSpec::configureOutput(std::string name, std::string &value)
{
  auto i = rs::find_if(io_.outputs_, [=](auto ns) { return ns.first == name; });
  value  = i->second.identifier();
}

void
    SubstrateSpec::bindEncoding(std::string name, EncodingSpec e)
{
  encodings_[name] = e;
}

void
    SubstrateSpec::configureEncoding(std::string name, EncodingSpec &e)
{
  e = encodings_[name];
}

void
    SubstrateSpec::bindNestedSubstrates(std::string                name,
                                        std::vector<SubstrateSpec> subs)
{
  if (nested_vector_.find(name) != nested_vector_.end())
  {
    std::cout << "User error: nested vector substrate " << name
              << " has already been declared\n";
    throw SpecError{};
  }
  nested_vector_[name];
  for (auto sub : subs)
  {
    NestedSubstrateSpec ns;
    ns.e = std::make_unique<SubstrateSpec>(sub);
    nested_vector_[name].push_back(ns);
  }
}

void
    SubstrateSpec::configureNestedSubstrates(std::string                 name,
                                             std::vector<SubstrateSpec> &subs)
{
  if (nested_vector_.find(name) == nested_vector_.end())
  {
    std::cout << "User error: nested vector substrate " << name
              << " has not been declared\n";
    throw SpecError{};
  }
  subs.clear();
  for (auto ns : nested_vector_[name])
  {
    auto ne = *ns.e;
    subs.push_back(ne);
  }
}

void
    SubstrateSpec::bindSubstrate(
        std::string                                      substrate_name,
        SubstrateSpec                                    sub,
        std::vector<std::pair<std::string, std::string>> input_constraints,
        std::vector<std::pair<std::string, std::string>> output_constraints)
{
  if (nested_.find(substrate_name) != nested_.end())
  {
    std::cout << "User error: nested substrate " << substrate_name
              << " has already been declared\n";
    throw SpecError{};
  }
  nested_[substrate_name].e = std::make_unique<SubstrateSpec>(sub);

  /*
  nested_[substrate_name].constraints_.inputs_ =
      input_constraints |
      rv::transform([](auto tag) -> std::pair<std::string, SignalSpec> {
        auto name  = tag.first;
        auto value = tag.second;
        return { name, SignalSpec{ name, name, value } };
      });

  nested_[substrate_name].constraints_.outputs_ =
      output_constraints |
      rv::transform([](auto tag) -> std::pair<std::string, SignalSpec> {
        auto name  = tag.first;
        auto value = tag.second;
        return { name, SignalSpec{ name, name, value } };
      });
  */ 
  rs::transform(input_constraints,
                rs::back_inserter(nested_[substrate_name].constraints_.inputs_),
                [](auto tag) -> std::pair<std::string, SignalSpec> {
                  auto name  = tag.first;
                  auto value = tag.second;
                  return { name, SignalSpec{ name, name, value } };
                });
  rs::transform(output_constraints,
                rs::back_inserter(nested_[substrate_name].constraints_.outputs_),
                [](auto tag) -> std::pair<std::string, SignalSpec> {
                  auto name  = tag.first;
                  auto value = tag.second;
                  return { name, SignalSpec{ name, name, value } };
                });
}

void
    SubstrateSpec::configureSubstrate(std::string name, SubstrateSpec &sub)
{
  if (!nested_[name].e)
  {
    std::cout << "Warning: <" << name_ << ":" << name
              << "> substrate spec has not been bind-ed (probably error)\n";
    //      std::exit(1);
  }
  else
    sub = *nested_[name].e;
}

void
    SubstrateSpec::errSignalBind(SubstrateSpec sub_spec,
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
    SubstrateSpec::matchNestedSignals(NestedSubstrateSpec &nes,
                                      SignalSpecSet &      source_tags,
                                      SignalSpecSet &      sink_tags,
                                      bool                 is_input)
{
  if (source_tags.size() != sink_tags.size())
  {
    std::cout << "cannot match nested substrate signals\n";
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
      errSignalBind(*nes.e, src, is_input);
    }

    rs::find_if(sink_tags,
                [sig = src](auto ns) { return ns.second.exactlyMatches(sig); })
        ->second.updateIdentifier(src.identifier());

    sink_tags_copy.erase(rs::find_if(sink_tags_copy, [sig = src](auto ns) {
      return ns.second.exactlyMatches(sig);
    }));
  }
}

void
    SubstrateSpec::checkNestedIO()
{
  for (auto &ename_es : nested_)
  {
    auto &es = ename_es.second;
    matchNestedSignals(es, es.constraints_.inputs_, es.e->io_.inputs_, true);
    matchNestedSignals(es, es.constraints_.outputs_, es.e->io_.outputs_, false);
    es.e->checkNestedIO();
  }
}

void
    SubstrateSpec::bindSubstrateIO(SubstrateSpec sub_spec)
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
      // throw;
    }
    if (!matches)
    {
      std::cout << "error: no input signals match exactly (convertible signals "
                   "not supported yet)\n  "
                << n_sig.second.fullName() << "\nviable candidates";
      for (auto sig : ios.inputs_)
        std::cout << "\n    " << sig.second.fullName();
      throw SpecError{};
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
      throw SpecError{};
    }
    auto i = rs::find_if(ios.outputs_, [sig = out_sig](auto ns) {
      return ns.second.exactlyMatches(sig);
    });
    out_sig.updateIdentifier(i->second.identifier());
    ios.outputs_.erase(i);
  }
}

void
    SubstrateSpec::parseParameters(language::Block block)
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
      errInvalidToken(
          value, "type mismatch, should be " + f->second.typeAsString());
      throw language::ParserError{};
    }
    f->second.parse(cp.valueAsString());
    auto con = f->second.checkConstraints();
    if (con)
    {
      errInvalidToken(value,
                             "parameter constraint not satisfied: " + *con);
      throw language::ParserError{};
    }
  }
}

void
    SubstrateSpec::parseNested( language::Block block)
{
  // Must be wrong
  for (auto blover : block.nested_)
  {
    auto name       = blover.first;
    auto nested_block = blover.second;

    auto ct = config_manager::typeOfBlock(nested_block.name_.substr(1));
    if (ct != config_manager::SpecType::substrate &&
        ct != config_manager::SpecType::encoding)
    {
      errInvalidToken(name,
                             "override of " + name.expr_ +
                                 " must be of type substrate/encoding");
      throw language::ParserError{};
    }

    if (ct == config_manager::SpecType::substrate)
    {
      auto f = rs::find_if(
          nested_, [&](auto param) { return param.first == name.expr_; });
      if (f == nested_.end())
      {
        errInvalidToken(
            name,
            "this does not override any nested substrates of " + block.name_,
          nested_ | rv::keys | rs::to<std::vector<std::string>>);
            //nested_ | rv::transform([](auto param) { return param.first; }));
        throw language::ParserError{};
      }

      f->second.e =
          std::make_unique<SubstrateSpec>(SubstrateSpec{  nested_block });
    }
    if (ct == config_manager::SpecType::encoding)
    {
      auto f = rs::find_if(
          encodings_, [&](auto param) { return param.first == name.expr_; });
      if (f == encodings_.end())
      {
        errInvalidToken(
            name,
            "this does not override any encodings of " + block.name_,
          encodings_ | rv::keys | rs::to<std::vector<std::string>>);
            //encodings_ | rv::transform([](auto param) { return param.first; }));
        throw language::ParserError{};
      }

      f->second = EncodingSpec{  nested_block };
    }
  }
}

void
    SubstrateSpec::parseNestedVector(
                                     language::Block  block)
{
  for (auto blover : block.nested_vector_)
  {
    auto name = blover.first;
    auto f    = rs::find_if(nested_vector_,
                         [&](auto param) { return param.first == name.expr_; });
    if (f == nested_vector_.end())
    {
      errInvalidToken(
          name,
          "this does not override any nested substrates of " + block.name_,
          nested_vector_ | rv::keys | rs::to<std::vector<std::string>>);
         // nested_vector_ |
           //   rv::transform([](auto param) { return param.first; }));
      throw language::ParserError{};
    }
    for (auto nested_block : blover.second)
    {

      if (config_manager::typeOfBlock(nested_block.name_.substr(1)) !=
          config_manager::SpecType::substrate)
      {
        errInvalidToken(name,
                               "nested vector of " + name.expr_ +
                                   " must be of type substrate");
        throw language::ParserError{};
      }

      NestedSubstrateSpec ns;
      ns.e =
          std::make_unique<SubstrateSpec>(SubstrateSpec{  nested_block });
      f->second.push_back(ns);
    }
  }
}

SubstrateSpec::SubstrateSpec( language::Block block)
{
  auto block_name = block.name_.substr(1);
  if (rs::none_of(config_manager::allSubstrateNames(),
                  [&](auto comp_name) { return comp_name == block_name; }))
  {
    errInvalidToken(block.name_token_,
                    "this is not an exisiting Substrate-component",
                    config_manager::allSubstrateNames());
    throw SpecError{};
  }

  *this = ALL_SUBSTRATE_SPECS.at(block_name);

  name_token_ = block.name_token_;

  parseParameters( block);
  parseNested( block);
  parseNestedVector( block);
  instantiateUserParameterSizes(0);
}

std::vector<std::string>
    SubstrateSpec::serialise(long depth) const
{
  std::vector<std::string> lines;
  auto                     alignment = std::string(depth, ' ');

  auto pad_signal = [&](auto sig) { return alignment + sig.second.fullName(); };

  lines.push_back(alignment + "substrate:" + name_);
  lines.push_back(alignment + "PARAMETERS");
  rs::transform(parameters_, rs::back_inserter(lines), [&](auto parameter) {
    return alignment + parameter.first + ":" + parameter.second.valueAsString();
  });
  lines.push_back(alignment + "INPUTS");
  rs::transform(io_.inputs_, rs::back_inserter(lines), pad_signal);
  lines.push_back(alignment + "OUTPUTS");
  rs::transform(io_.outputs_, rs::back_inserter(lines), pad_signal);
  lines.push_back(alignment + "NESTED");
  for (auto const &nested : nested_)
  {
    lines.push_back(alignment + nested.first);
    auto n_dump = nested.second.e->serialise(depth + 1);
    lines.insert(lines.end(), n_dump.begin(), n_dump.end());
  }
  lines.push_back(alignment + "NESTEDVECTOR");
  for (auto const &nested_vector : nested_vector_)
  {
    for (auto const &nested : nested_vector.second)
    {
      lines.push_back(alignment + nested_vector.first);
      auto n_dump = nested.e->serialise(depth + 1);
      lines.insert(lines.end(), n_dump.begin(), n_dump.end());
    }
  }
  lines.push_back(alignment + "ENCODINGS");
  for (auto const &encoding : encodings_)
  {
    lines.push_back(alignment + encoding.first);
    auto n_dump = encoding.second.serialise(depth + 1);
    lines.insert(lines.end(), n_dump.begin(), n_dump.end());
  }
  return lines;
}

SubstrateSpec
    SubstrateSpec::deserialise(std::vector<std::string> pop_dump)
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

  for (++f; *f != "NESTED"; f++)
  {
    auto l = *f;
    auto p = l.find(':');
    io_.outputs_.push_back({ l.substr(0, p), SignalSpec{ l } });
  }

  for (++f; *f != "NESTEDVECTOR";)
  {
    auto p =
        std::find_if(f + 1, pop_dump.end(), [](auto l) { return l[0] != ' '; });

    std::transform(f + 1, p, f + 1, [](auto l) { return l.substr(1); });

    SubstrateSpec e;
    nested_[*f].e = std::make_unique<SubstrateSpec>(
        e.deserialise(std::vector<std::string>(f + 1, pop_dump.end())));

    f = p;
  }

  for (++f; *f != "ENCODINGS";)
  {
    auto p =
        std::find_if(f + 1, pop_dump.end(), [](auto l) { return l[0] != ' '; });

    std::transform(f + 1, p, f + 1, [](auto l) { return l.substr(1); });

    SubstrateSpec       e;
    NestedSubstrateSpec ns;
    ns.e = std::make_unique<SubstrateSpec>(
        e.deserialise(std::vector<std::string>(f + 1, pop_dump.end())));
    nested_vector_[*f].push_back(ns);

    f = p;
  }

  for (++f; f != pop_dump.end();)
  {
    auto p =
        std::find_if(f + 1, pop_dump.end(), [](auto l) { return l[0] != ' '; });

    std::transform(f + 1, p, f + 1, [](auto l) { return l.substr(1); });

    EncodingSpec e;
    encodings_[*f] =
        e.deserialise(std::vector<std::string>(f + 1, pop_dump.end()));

    f = p;
  }

  return *this;
}

std::string
    SubstrateSpec::prettyPrint()
{
  std::stringstream out;
  out << "substrate::" << name_ << "\n{\n";

  out << " parameters\n";
  for (auto [parameter, value] : parameters_)
    out << std::setw(16) << parameter << " : " << value.valueAsString() << "\n";
  out << " inputs\n";
  for (auto [input, value] : io_.inputs_)
    out << std::setw(16) << input << " : " << value.type() << "\n";
  out << " outputs\n";
  for (auto [output, value] : io_.outputs_)
    out << std::setw(16) << output << " : " << value.type() << "\n";
  out << "}\n";
  return out.str();
}

void
    SubstrateSpec::instantiateUserParameterSizes(int sig_count)
{
  for (auto &n_sig : io_.inputs_)
    for (auto &[param, cp] : parameters_)
      if (cp.typeAsString() == "long" && param == n_sig.second.userParameter())
      {
        n_sig.second.instantiateUserParameter(std::stol(cp.valueAsString()));
        n_sig.second.updateIdentifier("~sig" + std::to_string(sig_count++));
      }
  for (auto &n_sig : io_.outputs_)
    for (auto &[param, cp] : parameters_)
      if (cp.typeAsString() == "long" && param == n_sig.second.userParameter())
      {
        n_sig.second.instantiateUserParameter(std::stol(cp.valueAsString()));
        n_sig.second.updateIdentifier("~sig" + std::to_string(sig_count++));
      }
}

}   // namespace specs
}   // namespace ded
