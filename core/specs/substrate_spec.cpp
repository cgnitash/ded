
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../configuration.h"
#include "../utilities/utilities.h"
#include "substrate_spec.h"

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
    SubstrateSpec::bindSubstrate(std::string name, SubstrateSpec sub)
{
  nested_[name].e = std::make_unique<SubstrateSpec>(sub);
}

void
    SubstrateSpec::configureSubstrate(std::string name, SubstrateSpec &sub)
{
  if (!nested_[name].e)
  {
    std::cout << "Warning: <" << name_ << ":" << name
              << "> environment spec has not been bind-ed (probably error)\n";
    //      std::exit(1);
  }
  else
    sub = *nested_[name].e;
}

void SubstrateSpec::bindSubstrateIO(SubstrateSpec sub_spec)
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
    SubstrateSpec::parseParameters(language::Parser parser,
                                   language::Block  block)
{
  for (auto over : block.overrides_)
  {
    auto name  = over.first;
    auto value = over.second;

    auto f = rs::find_if(parameters_,
                         [&](auto param) { return param.first == name.expr_; });
    if (f == parameters_.end())
    {
      parser.errInvalidToken(
          name,
          "this does not override any parameters of " + name_,
          parameters_ | rv::transform([](auto param) { return param.first; }));
      throw language::ParserError{};
    }

    ConfigurationPrimitive cp;
    cp.parse(value.expr_);
    if (cp.typeAsString() != f->second.typeAsString())
    {
      parser.errInvalidToken(
          value, "type mismatch, should be " + f->second.typeAsString());
      throw language::ParserError{};
    }
    f->second.parse(cp.valueAsString());
    auto con = f->second.checkConstraints();
    if (con)
    {
      parser.errInvalidToken(value,
                             "parameter constraint not satisfied: " + *con);
      throw language::ParserError{};
    }
  }
}

void
    SubstrateSpec::parseNested(language::Parser parser, language::Block block)
{
  // Must be wrong
  for (auto blover : block.nested_)
  {
    auto name       = blover.first;
    auto nested_blk = blover.second;

    auto ct = config_manager::typeOfBlock(nested_blk.name_.substr(1));
    if (ct != "substrate" && ct != "encoding")
    {
      parser.errInvalidToken(name,
                             "override of " + name.expr_ +
                                 " must be of type substrate/encoding");
      throw language::ParserError{};
    }

    if (ct == "substrate")
    {
      auto f = rs::find_if(
          nested_, [&](auto param) { return param.first == name.expr_; });
      if (f == nested_.end())
      {
        parser.errInvalidToken(
            name,
            "this does not override any nested substrates of " + block.name_,
            nested_ | rv::transform([](auto param) { return param.first; }));
        throw language::ParserError{};
      }

      f->second.e =
          std::make_unique<SubstrateSpec>(SubstrateSpec{ parser, nested_blk });
    }
    if (ct == "encoding")
    {
      auto f = rs::find_if(
          encodings_, [&](auto param) { return param.first == name.expr_; });
      if (f == encodings_.end())
      {
        parser.errInvalidToken(
            name,
            "this does not override any encodings of " + block.name_,
            encodings_ | rv::transform([](auto param) { return param.first; }));
        throw language::ParserError{};
      }

      f->second = EncodingSpec{ parser, nested_blk };
    }
  }
}

void
    SubstrateSpec::parseNestedVector(language::Parser parser,
                                     language::Block  block)
{
  for (auto blover : block.nested_vector_)
  {
    auto name = blover.first;
    auto f    = rs::find_if(nested_vector_,
                         [&](auto param) { return param.first == name.expr_; });
    if (f == nested_vector_.end())
    {
      parser.errInvalidToken(
          name,
          "this does not override any nested substrates of " + block.name_,
          nested_vector_ |
              rv::transform([](auto param) { return param.first; }));
      throw language::ParserError{};
    }
    for (auto nested_blk : blover.second)
    {

      auto ct = config_manager::typeOfBlock(nested_blk.name_.substr(1));
      if (ct != "substrate")
      {
        parser.errInvalidToken(name,
                               "nested vector of " + name.expr_ +
                                   " must be of type substrate");
        throw language::ParserError{};
      }

      NestedSubstrateSpec ns;
      ns.e =
          std::make_unique<SubstrateSpec>(SubstrateSpec{ parser, nested_blk });
      f->second.push_back(ns);
    }
  }
}

SubstrateSpec::SubstrateSpec(language::Parser parser, language::Block block)
{
  *this =  ALL_SUBSTRATE_SPECS[block.name_.substr(1)];

  name_token_ = block.name_token_;

  parseParameters(parser, block);
  parseNested(parser, block);
  parseNestedVector(parser, block);
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
    out << std::setw(16) << input << " : " << value.fullName() << "\n";
  out << " outputs\n";
  for (auto [output, value] : io_.outputs_)
    out << std::setw(16) << output << " : " << value.fullName() << "\n";
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