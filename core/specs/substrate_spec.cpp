
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

SubstrateSpec::SubstrateSpec(language::Parser p, language::Block blk)
{

  auto t = ALL_SUBSTRATE_SPECS[blk.name_.substr(1)];

  *this = t;

  for (auto over : blk.overrides_)
  {
    auto name  = over.first;
    auto value = over.second;

    auto f = rs::find_if(parameters_,
                         [&](auto param) { return param.first == name.expr_; });
    if (f == parameters_.end())
    {
      p.errInvalidToken(
          name,
          "this does not override any parameters of " + name_,
          parameters_ | rv::transform([](auto param) { return param.first; }));
      throw language::ParserError{};
    }

    ConfigurationPrimitive cp;
    cp.parse(value.expr_);
    if (cp.typeAsString() != f->second.typeAsString())
    {
      p.errInvalidToken(value,
                        "type mismatch, should be " + f->second.typeAsString());
      throw language::ParserError{};
    }
    f->second.parse(cp.valueAsString());
    auto con = f->second.checkConstraints();
    if (con)
    {
      p.errInvalidToken(value, "parameter constraint not satisfied: " + *con);
      throw language::ParserError{};
    }
  }

  for (auto blover : blk.nested_)
  {
    auto name       = blover.first;
    auto nested_blk = blover.second;

    auto ct = config_manager::typeOfBlock(nested_blk.name_.substr(1));
    if (ct != "substrate" && ct != "encoding")
    {
      p.errInvalidToken(name,
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
        p.errInvalidToken(
            name,
            "this does not override any nested substrates of " + blk.name_,
            nested_ | rv::transform([](auto param) { return param.first; }));
        throw language::ParserError{};
      }

      f->second.e =
          std::make_unique<SubstrateSpec>(SubstrateSpec{ p, nested_blk });
    }
    if (ct == "encoding")
    {
      auto f = rs::find_if(
          encodings_, [&](auto param) { return param.first == name.expr_; });
      if (f == encodings_.end())
      {
        p.errInvalidToken(
            name,
            "this does not override any encodings of " + blk.name_,
            encodings_ | rv::transform([](auto param) { return param.first; }));
        throw language::ParserError{};
      }

      f->second = EncodingSpec{ p, nested_blk };
    }
  }
}

std::vector<std::string>
    SubstrateSpec::serialise(long depth) const
{
  std::vector<std::string> lines;
  auto                     alignment = std::string(depth, ' ');

  auto pad_signal = [&](auto sig) { return alignment + sig.second.fullName(); };

  lines.push_back(alignment + "substrate:" + name_);
  lines.push_back(alignment + "P");
  rs::transform(parameters_, rs::back_inserter(lines), [&](auto parameter) {
    return alignment + parameter.first + ":" + parameter.second.valueAsString();
  });
  lines.push_back(alignment + "I");
  rs::transform(io_.inputs_, rs::back_inserter(lines), pad_signal);
  lines.push_back(alignment + "O");
  rs::transform(io_.outputs_, rs::back_inserter(lines), pad_signal);
  lines.push_back(alignment + "n");
  for (auto const &nested : nested_)
  {
    lines.push_back(alignment + nested.first);
    auto n_dump = nested.second.e->serialise(depth + 1);
    lines.insert(lines.end(), n_dump.begin(), n_dump.end());
  }
  lines.push_back(alignment + "m");
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

  for (++f; *f != "n"; f++)
  {
    auto l = *f;
    auto p = l.find(':');
    io_.outputs_.push_back({ l.substr(0, p), SignalSpec{ l } });
  }

  for (++f; *f != "m";)
  {
    auto p =
        std::find_if(f + 1, pop_dump.end(), [](auto l) { return l[0] != ' '; });

    std::transform(f + 1, p, f + 1, [](auto l) { return l.substr(1); });

    SubstrateSpec e;
    nested_[*f].e = std::make_unique<SubstrateSpec>(
        e.deserialise(std::vector<std::string>(f + 1, pop_dump.end())));

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

IO
    SubstrateSpec::instantiateUserParameterSizes(int sig_count)
{
  for (auto &n_sig : io_.inputs_)
    for (auto &[param, cp] : parameters_)
      if (cp.typeAsString() == "long" && param == n_sig.second.userParameter())
      {
        n_sig.second.instantiateUserParameter(std::stol(cp.valueAsString()));
        n_sig.second.updateIdentifier("sig" + std::to_string(sig_count++));
      }
  for (auto &n_sig : io_.outputs_)
    for (auto &[param, cp] : parameters_)
      if (cp.typeAsString() == "long" && param == n_sig.second.userParameter())
      {
        n_sig.second.instantiateUserParameter(std::stol(cp.valueAsString()));
        n_sig.second.updateIdentifier("sig" + std::to_string(sig_count++));
      }
  return io_;
}

}   // namespace specs
}   // namespace ded
