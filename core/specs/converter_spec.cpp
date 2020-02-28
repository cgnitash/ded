
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../configuration.hpp"
#include "converter_spec.hpp"
//#include "substrate_spec.hpp"

namespace ded
{
namespace specs
{

ConverterSpec::ConverterSpec( language::Block block)
{
  auto block_name = block.name_.substr(1);
  if (!rs::contains(config_manager::allConverterNames(), block_name))
  {
    errInvalidToken(block.name_token_,
                    "this is not an exisiting Converter-component",
                    config_manager::allConverterNames());
    throw SpecError{};
  }

  *this = ALL_CONVERTER_SPECS.at(block_name);

  parameters_.loadFromSpec(block.overrides_, name_);
  /*
  for (auto blover : block.nested_)
  {
    auto name       = blover.first;
    auto nested_block = blover.second;

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
  */
}

std::vector<std::string>
    ConverterSpec::serialise(long depth) const
{
  std::vector<std::string> lines;
  auto                     alignment = std::string(depth, ' ');

  lines.push_back(alignment + "converter:" + name_);
  lines.push_back(alignment + "PARAMETERS");
  rs::transform(parameters_.parameters_, rs::back_inserter(lines), [&](auto parameter) {
    return alignment + parameter.first + ":" + parameter.second.valueAsString();
  });
  /*
  lines.push_back(alignment + "ENTITY");
  auto n_dump = es_.serialise(depth + 1);
  lines.insert(lines.end(), n_dump.begin(), n_dump.end());
  */
  return lines;
}

ConverterSpec
    ConverterSpec::deserialise(std::vector<std::string> pop_dump)
{
  name_ = *pop_dump.begin();
  name_ = name_.substr(name_.find(':') + 1);

  auto f = rs::begin(pop_dump) + 2;

  for (; f != pop_dump.end(); f++)
  {
    auto                   l = *f;
    auto                   p = l.find(':');
    ConfigurationPrimitive c;
    c.parse(l.substr(p + 1));
    parameters_.parameters_[l.substr(0, p)] = c;
  }

  /*
  for (auto l = (++f) + 1; l != pop_dump.end(); l++)
  {
    l->erase(0, 1);
  }

  SubstrateSpec es;
  es_ = es.deserialise(std::vector<std::string>(f, pop_dump.end()));
	*/

  ConverterSpec ps = *this;
  return ps;
}

std::string
    ConverterSpec::prettyPrint()
{
  std::stringstream out;
  out << "converter::" << name_ << "\n{\n";

  out << " parameters\n";
  for (auto [parameter, value] : parameters_.parameters_)
    out << std::setw(16) << parameter << " : " << value.valueAsString() << "\n";

  out << std::setw(16) << "input" << " : " << args_.first.type() << "\n";
  out << std::setw(16) << "output" << " : " << args_.second.type() << "\n";
  out << "}\n";
  return out.str();
}

/*
ConverterSink parseConversionSequence(language::Token token){

	auto expression = token.expr_.substr(1, token.expr_.size() - 2);

	auto lst = expression | rv::split('>') | rs::to<std::vector<std::string>>;

    auto sink = lst.back();
	
    lst.pop_back();

	for (auto &convert : lst)
		;	


}
*/

}   // namespace specs
}   // namespace ded
