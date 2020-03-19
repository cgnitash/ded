
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../configuration.hpp"
#include "population_spec.hpp"
#include "substrate_spec.hpp"

namespace ded
{
namespace specs
{

void
    PopulationSpec::checkInvalidTokens(language::Block block)
{

  if (!block.traces_.empty())
  {
    errInvalidToken(block.traces_[0].lhs_,
                    "traces are not allowed in Populations");
    throw SpecError{};
  }

  if (!block.input_signal_binds_.empty())
  {
    errInvalidToken(block.input_signal_binds_[0].sink_,
                    "signal bindings are not allowed in Populations");
    throw SpecError{};
  }

  if (!block.output_signal_binds_.empty())
  {
    errInvalidToken(block.output_signal_binds_[0].sink_,
                    "signal bindings are not allowed in Populations");
    throw SpecError{};
  }

  if (!block.tag_binds_.empty())
  {
    errInvalidToken(block.tag_binds_[0].sink_,
                    "signal bindings are not allowed in Populations");
    throw SpecError{};
  }

  if (!block.nested_vector_.empty())
  {
    errInvalidToken(block.nested_vector_[0].name_,
                    "Converters can't have nested component vectors");
    throw SpecError{};
  }
}

PopulationSpec::PopulationSpec( language::Block block)
{
  auto block_name = block.name_.substr(1);
  if (!rs::contains(config_manager::allPopulationNames(), block_name))
  {
    errInvalidToken(block.name_token_,
                    "this is not an exisiting Population-component",
                    config_manager::allPopulationNames());
    throw SpecError{};
  }

  *this = ALL_POPULATION_SPECS.at(block_name);

  checkInvalidTokens(block);

  parameters_.loadFromSpec(block.overrides_, name_);

  for (auto blover : block.nested_)
  {
    auto name       = blover.name_;
    auto nested_block = blover.blocks_[0];

    if (!config_manager::isSubstrateBlock(nested_block.name_.substr(1)))
    {
      errInvalidToken(name,
                             "override of " + name.expr_ +
                                 " inside population:: must be of type substrate");
      throw language::ParserError{};
    }

    es_ = SubstrateSpec{ nested_block };
  }
}

std::vector<std::string>
    PopulationSpec::serialise(long depth) const
{
  std::vector<std::string> lines;
  auto                     alignment = std::string(depth, ' ');

  lines.push_back(alignment + "population:" + name_);
  lines.push_back(alignment + "PARAMETERS");
  rs::transform(
      parameters_.parameters_, rs::back_inserter(lines), [&](auto parameter) {
        return alignment + parameter.first + ":" +
               parameter.second.valueAsString();
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
    parameters_.parameters_[l.substr(0, p)] = c;
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
  for (auto [parameter, value] : parameters_.parameters_)
    out << std::setw(16) << parameter << " : " << value.valueAsString() << "\n";

  out << "}\n";
  return out.str();
}
}   // namespace specs
}   // namespace ded
