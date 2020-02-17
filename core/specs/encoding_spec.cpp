
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../configuration.hpp"
#include "../utilities/utilities.hpp"
#include "encoding_spec.hpp"

namespace ded
{
namespace specs
{


EncodingSpec::EncodingSpec( language::Block block)
{

  auto block_name = block.name_.substr(1);
  if (!rs::contains(config_manager::allEncodingNames(), block_name))
  {
    errInvalidToken(block.name_token_,
                    "this is not an exisiting Encoding-component",
                    config_manager::allEncodingNames());
    throw SpecError{};
  }

  *this = ALL_ENCODING_SPECS.at(block_name);

  parameters_.loadFromSpec(block.overrides_, name_);
}

std::vector<std::string>
    EncodingSpec::serialise(long depth) const
{
  std::vector<std::string> lines;
  auto alignment = std::string(depth, ' ');

  lines.push_back(alignment + "encoding:" + name_);
  lines.push_back(alignment + "P");
  rs::transform(
      parameters_.parameters_, rs::back_inserter(lines), [&](auto parameter) {
        return alignment + parameter.first + ":" +
               parameter.second.valueAsString();
      });
  return lines;
}

EncodingSpec
    EncodingSpec::deserialise(std::vector<std::string> pop_dump)
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

  return *this;
}

std::string
    EncodingSpec::prettyPrint()
{
  std::stringstream out;
  out << "encoding::" << name_ << "\n{\n";

  out << " parameters\n";
  for (auto [parameter, value] : parameters_.parameters_)
    out << std::setw(16) << parameter << " : " << value.valueAsString()
        << "\n";
  out << "}\n";
  return out.str();
}
}   // namespace specs
}   // namespace ded
