
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../configuration.h"
#include "entity_spec.h"
#include "population_spec.h"

namespace ded
{
namespace specs
{

PopulationSpec::PopulationSpec(language::Parser parser, language::Block block)
{

  *this = all_population_specs[block.name_.substr(1)];

  for (auto over : block.overrides_)
  {
    auto name  = over.first;
    auto value = over.second;

    auto f = rs::find_if(
        parameters_, [&](auto param) { return param.first == name.expr_; });
    if (f == parameters_.end())
    {
      parser.errInvalidToken(name,
                          "this does not override any parameters of " + name_,
                          parameters_ | rv::transform([](auto param) {
                            return param.first;
                          }));
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
      parser.errInvalidToken(
          value,
              "parameter constraint not satisfied: " + *con );
      throw language::ParserError{};
    }
  }

  for (auto blover : block.nested_)
  {
    auto name       = blover.first;
    auto nested_blk = blover.second;

    auto ct = config_manager::typeOfBlock(nested_blk.name_.substr(1));
    if (ct != "entity")
    {
      parser.errInvalidToken(name,
                          "override of " + name.expr_ +
                              " inside population:: must be of type entity");
      throw language::ParserError{};
    }

    es_ = EntitySpec{ parser, nested_blk };
  }
}

std::string
    PopulationSpec::dump(long depth) const
{
  auto alignment = std::string(depth, ' ');

  return alignment + "population:" + name_ + "\n" + alignment + "P\n" +
         (parameters_ | rv::transform([&](auto parameter) {
            return alignment + parameter.first + ":" +
                   parameter.second.valueAsString() + "\n";
          }) |
          ra::join)+
         alignment + "E" + es_.dump(depth + 1);
}

PopulationSpec
    PopulationSpec::parse(std::vector<std::string> pop_dump)
{
  name_ = *pop_dump.begin();
  name_ = name_.substr(name_.find(':') + 1);

  auto f = rs::begin(pop_dump) + 2;

  for (; *f != "E"; f++)
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

  EntitySpec es;
  es_ = es.parse(std::vector<std::string>(f, pop_dump.end()));

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
    out << std::setw(16) << parameter << " : " << value.valueAsString()
        << "\n";
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
