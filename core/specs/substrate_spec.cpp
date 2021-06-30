
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
    SubstrateSpec::checkInvalidTokens(language::Block block)
{

  if (!block.traces_.empty())
  {
    errInvalidToken(block.traces_[0].lhs_,
                    "traces are not allowed in Substrates");
    throw SpecError{};
  }

  if (!block.input_signal_binds_.empty())
  {
    errInvalidToken(block.input_signal_binds_[0].sink_,
                    "signal bindings are not allowed in Substrates");
    throw SpecError{};
  }

  if (!block.output_signal_binds_.empty())
  {
    errInvalidToken(block.output_signal_binds_[0].sink_,
                    "signal bindings are not allowed in Substrates");
    throw SpecError{};
  }

  if (!block.tag_binds_.empty())
  {
    errInvalidToken(block.tag_binds_[0].sink_,
                    "signal bindings are not allowed in Substrates");
    throw SpecError{};
  }
}

void
    SubstrateSpec::input(std::string name, std::string type)
{
  io_.inputs_.push_back({ name, SignalSpec{ type } });
}

void
    SubstrateSpec::output(std::string name, std::string type)
{
  io_.outputs_.push_back({ name, SignalSpec{ type } });
}

void
    SubstrateSpec::encoding(std::string name, EncodingSpec &e)
{
  if (isConfigurable)
    e = encodings_[name];
  else
    encodings_[name] = e;
}

void
    SubstrateSpec::nestedSubstrateVector(std::string                 name,
                                         std::vector<SubstrateSpec> &subs)
{
  if (isConfigurable)
    configureNestedSubstrateVector(name, subs);
  else
    bindNestedSubstrateVector(name, subs);
}

void
    SubstrateSpec::bindNestedSubstrateVector(std::string                name,
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
    SubstrateSpec::configureNestedSubstrateVector(
        std::string                 name,
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
    SubstrateSpec::nestedSubstrate(
        std::string                   name,
        SubstrateSpec &               sub,
        std::vector<SignalConstraint> input_constraints,
        std::vector<SignalConstraint> output_constraints)
{
  if (isConfigurable)
    configureSubstrate(name, sub);
  else
    bindSubstrate(name, sub, input_constraints, output_constraints);
}

void
    SubstrateSpec::bindSubstrate(
        std::string                   substrate_name,
        SubstrateSpec                 sub,
        std::vector<SignalConstraint> input_constraints,
        std::vector<SignalConstraint> output_constraints)
{
  if (nested_.find(substrate_name) != nested_.end())
  {
    std::cout << "User error: nested substrate " << substrate_name
              << " has already been declared\n";
    throw SpecError{};
  }
  nested_[substrate_name].e = std::make_unique<SubstrateSpec>(sub);

  auto &constraints = nested_[substrate_name].constraints_;

  rs::transform(
      input_constraints, rs::back_inserter(constraints.inputs_), toSignal);

  rs::transform(
      output_constraints, rs::back_inserter(constraints.outputs_), toSignal);
}

void
    SubstrateSpec::configureSubstrate(std::string name, SubstrateSpec &sub)
{
  if (!nested_[name].e)
  {
    std::cout << "Warning: <" << name_ << ":" << name
              << "> substrate spec has not been bind-ed (probably error)\n";
  }
  else
    sub = *nested_[name].e;
}

void
    SubstrateSpec::parseNested(language::Block block)
{
  // Must be wrong
  for (auto blover : block.nested_)
  {
    auto name         = blover.name_;
    auto nested_block = blover.blocks_[0];

    if (!config_manager::isSubstrateBlock(nested_block.name_.substr(1)) &&
        !config_manager::isEncodingBlock(nested_block.name_.substr(1)))
    {
      errInvalidToken(name,
                      "override of " + name.expr_ +
                          " must be of type substrate/encoding");
      throw language::ParserError{};
    }

    if (config_manager::isSubstrateBlock(nested_block.name_.substr(1)))
    {
      auto f = rs::find_if(
          nested_, [&](auto param) { return param.first == name.expr_; });
      if (f == nested_.end())
      {
        errInvalidToken(name,
                        "this does not override any nested substrates of " +
                            block.name_,
                        nested_ | rv::keys | rs::to<std::vector<std::string>>);
        throw language::ParserError{};
      }

      f->second.e =
          std::make_unique<SubstrateSpec>(SubstrateSpec{ nested_block });
    }
    if (config_manager::isEncodingBlock(nested_block.name_.substr(1)))
    {
      auto f = rs::find_if(
          encodings_, [&](auto param) { return param.first == name.expr_; });
      if (f == encodings_.end())
      {
        errInvalidToken(
            name,
            "this does not override any encodings of " + block.name_,
            encodings_ | rv::keys | rs::to<std::vector<std::string>>);
        throw language::ParserError{};
      }

      f->second = EncodingSpec{ nested_block };
    }
  }
}

void
    SubstrateSpec::parseNestedVector(language::Block block)
{
  for (auto blover : block.nested_vector_)
  {
    auto name = blover.name_;
    auto f    = rs::find_if(nested_vector_,
                         [&](auto param) { return param.first == name.expr_; });
    if (f == nested_vector_.end())
    {
      errInvalidToken(
          name,
          "this does not override any nested substrates of " + block.name_,
          nested_vector_ | rv::keys | rs::to<std::vector<std::string>>);
      throw language::ParserError{};
    }
    for (auto nested_block : blover.blocks_)
    {
      if (!config_manager::isSubstrateBlock(nested_block.name_.substr(1)))
      {
        errInvalidToken(name,
                        "nested vector of " + name.expr_ +
                            " must be of type substrate");
        throw language::ParserError{};
      }

      NestedSubstrateSpec ns;
      ns.e = std::make_unique<SubstrateSpec>(SubstrateSpec{ nested_block });
      f->second.push_back(ns);
    }
  }
}

SubstrateSpec::SubstrateSpec(language::Block block)
{
  auto block_name = block.name_.substr(1);
  if (!rs::contains(config_manager::allSubstrateNames(), block_name))
  {
    errInvalidToken(block.name_token_,
                    "this is not an exisiting Substrate-component",
                    config_manager::allSubstrateNames());
    throw SpecError{};
  }

  *this = ALL_SUBSTRATE_SPECS.at(block_name);

  checkInvalidTokens(block);

  name_token_ = block.name_token_;

  parameters_.loadFromSpec(block.overrides_, name_);
  parseNested(block);
  parseNestedVector(block);
  instantiateUserParameterSizes();
}

std::vector<std::string>
    SubstrateSpec::serialise(long depth) const
{
  std::vector<std::string> lines;
  auto                     alignment = std::string(depth, ' ');

  auto pad_signal = [&](auto sig) { return alignment + sig.name_; };

  lines.push_back(alignment + "substrate:" + name_);
  lines.push_back(alignment + "PARAMETERS");
  rs::transform(
      parameters_.parameters_, rs::back_inserter(lines), [&](auto parameter) {
        return alignment + parameter.first + ":" +
               parameter.second.valueAsString();
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
    parameters_.parameters_[l.substr(0, p)] = c;
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
  for (auto [parameter, value] : parameters_.parameters_)
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
    SubstrateSpec::instantiateUserParameterSizes()
{
  for (auto &n_sig : io_.inputs_)
    n_sig.signal_spec_.instantiateUserParameter(parameters_.parameters_);

  for (auto &n_sig : io_.outputs_)
    n_sig.signal_spec_.instantiateUserParameter(parameters_.parameters_);
}

}   // namespace specs
}   // namespace ded
