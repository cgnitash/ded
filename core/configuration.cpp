

#include <experimental/filesystem>
#include <regex>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "utilities/term_colours.hpp"
#include "utilities/utilities.hpp"
#include "configuration.hpp"

namespace ded
{
namespace config_manager
{

SpecType
    typeOfBlock(std::string name)
{
  if (isSubstrateBlock(name))
    return SpecType::substrate;
  if (isProcessBlock(name))
    return SpecType::process;
  if (isPopulationBlock(name))
    return SpecType::population;
  if (isEncodingBlock(name))
    return SpecType::encoding;
  return SpecType::UNKNOWN;
}

void
    showConfig(std::ostream &out, std::string name)
{
  switch (typeOfBlock(name))
  {
    case SpecType::process:
      out << ALL_PROCESS_SPECS[name].prettyPrint();
      break;
    case SpecType::substrate:
      out << ALL_SUBSTRATE_SPECS[name].prettyPrint();
      break;
    case SpecType::population:
      out << ALL_POPULATION_SPECS[name].prettyPrint();
      break;
    case SpecType::encoding:
      out << ALL_ENCODING_SPECS[name].prettyPrint();
      break;
    case SpecType::UNKNOWN:
      out << "component " << name << " not found\n";
  }

  return;
}

void
    listAllConfigs(std::ostream &out)
{
  out << "Substrate:\n";
  for (auto n_spec : ALL_SUBSTRATE_SPECS)
    out << "    " << n_spec.first << "\n";

  out << "Process:\n";
  for (auto n_spec : ALL_PROCESS_SPECS)
    out << "    " << n_spec.first << "\n";

  out << "Population:\n";
  for (auto n_spec : ALL_POPULATION_SPECS)
    out << "    " << n_spec.first << "\n";

  out << "Encoding:\n";
  for (auto n_spec : ALL_ENCODING_SPECS)
    out << "    " << n_spec.first << "\n";

  return;
}

void
    saveAllConfigs()
{
  std::ofstream file("configurations.cfg");
  for (auto n_es : ALL_SUBSTRATE_SPECS)
    file << "\n" << n_es.second.prettyPrint();
  for (auto n_es : ALL_PROCESS_SPECS)
    file << "\n" << n_es.second.prettyPrint();
  for (auto n_es : ALL_POPULATION_SPECS)
    file << "\n" << n_es.second.prettyPrint();
  for (auto n_es : ALL_ENCODING_SPECS)
    file << "\n" << n_es.second.prettyPrint();
  return;
}

bool
    isSubstrateBlock(std::string name)
{
  return ALL_SUBSTRATE_SPECS.find(name) != ALL_SUBSTRATE_SPECS.end();
}

bool        isEncodingBlock(std::string name)
{
  return ALL_ENCODING_SPECS.find(name) != ALL_ENCODING_SPECS.end();
}

bool        isProcessBlock(std::string name)
{
  return ALL_PROCESS_SPECS.find(name) != ALL_PROCESS_SPECS.end();
}

bool        isPopulationBlock(std::string name)
{
  return ALL_POPULATION_SPECS.find(name) != ALL_POPULATION_SPECS.end();
}

std::vector<std::string>
    allSubstrateNames()
{
  return ALL_SUBSTRATE_SPECS |
         rv::transform([](auto spec) { return spec.first; });
}

std::vector<std::string>
    allProcessNames()
{
  return ALL_PROCESS_SPECS |
         rv::transform([](auto spec) { return spec.first; });
}

std::vector<std::string>
    allPopulationNames()
{
  return ALL_POPULATION_SPECS |
         rv::transform([](auto spec) { return spec.first; });
}

std::vector<std::string>
    allEncodingNames()
{
  return ALL_ENCODING_SPECS |
         rv::transform([](auto spec) { return spec.first; });
}

std::vector<std::string>
    allComponentNames()
{
  auto a = allSubstrateNames();
  auto b = allProcessNames();
  auto c = allPopulationNames();
  auto d = allEncodingNames();
  return rv::concat(a, b, c, d);
}

}   // namespace config_manager
}   // namespace ded
