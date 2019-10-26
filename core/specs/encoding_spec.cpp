
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
  if (rs::none_of(config_manager::allEncodingNames(),
                  [&](auto comp_name) { return comp_name == block_name; }))
  {
    errInvalidToken(block.name_token_,
                    "this is not an exisiting Encoding-component",
                    config_manager::allEncodingNames());
    throw SpecError{};
  }

  *this = ALL_ENCODING_SPECS.at(block_name);

  for (auto over : block.overrides_)
  {
    auto name  = over.first;
    auto value = over.second;

    auto f = rs::find_if(
        parameters_, [&](auto param) { return param.first == name.expr_; });
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
      errInvalidToken(
          value,
              "parameter constraint not satisfied: " + *con );
      throw language::ParserError{};
    }
  }

}

std::vector<std::string>
    EncodingSpec::serialise(long depth) const
{
  std::vector<std::string> lines;
  auto alignment = std::string(depth, ' ');

  lines.push_back(alignment + "encoding:" + name_);
  lines.push_back(alignment + "P");
  rs::transform(
      parameters_, rs::back_inserter(lines), [&](auto parameter) {
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
    parameters_[l.substr(0, p)] = c;
  }

  return *this;
}

std::string
    EncodingSpec::prettyPrint()
{
  std::stringstream out;
  out << "encoding::" << name_ << "\n{\n";

  out << " parameters\n";
  for (auto [parameter, value] : parameters_)
    out << std::setw(16) << parameter << " : " << value.valueAsString()
        << "\n";
  out << "}\n";
  return out.str();
}
}   // namespace specs
}   // namespace ded
