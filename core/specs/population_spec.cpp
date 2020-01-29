
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

  for (auto over : block.overrides_)
  {
    auto name  = over.lhs_;
    auto value = over.rhs_;

    auto f = rs::find_if(parameters_,
                         [&](auto param) { return param.first == name.expr_; });
    if (f == parameters_.end())
    {
      errInvalidToken(name,
                      "this does not override any parameters of " + name_,
                      parameters_ | rv::keys |
                          rs::to<std::vector<std::string>>);
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

  for (auto blover : block.nested_)
  {
    auto name       = blover.name_;
    auto nested_block = blover.blocks_[0];

    if (config_manager::typeOfBlock(nested_block.name_.substr(1)) !=
        config_manager::SpecType::substrate)
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

  out << "}\n";
  return out.str();
}
}   // namespace specs
}   // namespace ded
