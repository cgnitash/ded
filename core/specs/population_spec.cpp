
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../configuration.h"
#include "population_spec.h"
#include "substrate_spec.h"

namespace ded
{
namespace specs
{

PopulationSpec::PopulationSpec(language::Parser parser, language::Block block)
{

  *this = ALL_POPULATION_SPECS[block.name_.substr(1)];

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

  for (auto blover : block.nested_)
  {
    auto name       = blover.first;
    auto nested_blk = blover.second;

    auto ct = config_manager::typeOfBlock(nested_blk.name_.substr(1));
    if (ct != "substrate")
    {
      parser.errInvalidToken(name,
                             "override of " + name.expr_ +
                                 " inside population:: must be of type substrate");
      throw language::ParserError{};
    }

    es_ = SubstrateSpec{ parser, nested_blk };
  }
}

std::vector<std::string>
    PopulationSpec::serialise(long depth) const
{
  std::vector<std::string> lines;
  auto                     alignment = std::string(depth, ' ');

  lines.push_back(alignment + "population:" + name_);
  lines.push_back(alignment + "PARAMETERS");
  rs::transform(parameters_, rs::back_inserter(lines), [&](auto parameter) {
    return alignment + parameter.first + ":" + parameter.second.valueAsString();
  });
  lines.push_back(alignment + "ENTITY");
  auto n_dump = es_.serialise(depth + 1);
  lines.insert(lines.end(), n_dump.begin(), n_dump.end());
  return lines;
}

PopulationSpec
    PopulationSpec::deserialise(std::vector<std::string> pop_dump)
{
  name_ = *pop_dump.begin();
  name_ = name_.substr(name_.find(':') + 1);

  auto f = rs::begin(pop_dump) + 2;

  for (; *f != "ENTITY"; f++)
  {
    auto                   l = *f;
    auto                   p = l.find(':');
    ConfigurationPrimitive c;
    c.parse(l.substr(p + 1));
    parameters_[l.substr(0, p)] = c;
  }

  for (auto l = (++f) + 1; l != pop_dump.end(); l++)
  {
    l->erase(0, 1);
  }

  SubstrateSpec es;
  es_ = es.deserialise(std::vector<std::string>(f, pop_dump.end()));

  PopulationSpec ps = *this;
  return ps;
}

std::string
    PopulationSpec::prettyPrint()
{
  std::stringstream out;
  out << "population::" << name_ << "\n{\n";

  out << " parameters\n";
  for (auto [parameter, value] : parameters_)
    out << std::setw(16) << parameter << " : " << value.valueAsString() << "\n";
  /*
out << TermColours::yellow_fg << "inputs----" << TermColours::reset
  << "\n";
for (auto [input, value] : inputs_)
out << std::setw(26) << input << " : " << value << "\n";
out << TermColours::yellow_fg << "outputs----" << TermColours::reset
  << "\n";
for (auto [output, value] : outputs_)
out << std::setw(26) << output << " : " << value << "\n";
    */

  out << "}\n";
  return out.str();
}
}   // namespace specs
}   // namespace ded
