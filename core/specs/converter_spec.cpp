
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../configuration.hpp"
#include "converter_spec.hpp"

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

ConversionSignature
    makeSliceConverter(long from, long every, std::string vtt)
{
  if (vtt == "double")
    return sliceConverter<double>(from, every);
  if (vtt == "long")
    return sliceConverter<long>(from, every);
  if (vtt == "bool")
    return sliceConverter<bool>(from, every);
  return sliceConverter<std::string>(from, every);
}
}   // namespace specs
}   // namespace ded
