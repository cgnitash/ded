

#include <experimental/filesystem>
#include <regex>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "utilities/term_colours.h"
#include "utilities/utilities.h"
#include "configuration.h"

namespace ded
{
namespace config_manager
{

std::string
    typeOfBlock(std::string name)
{
  if (ALL_PROCESS_SPECS.find(name) != ALL_PROCESS_SPECS.end())
    return "environment";
  if (ALL_SUBSTRATE_SPECS.find(name) != ALL_SUBSTRATE_SPECS.end())
    return "entity";
  if (ALL_POPULATION_SPECS.find(name) != ALL_POPULATION_SPECS.end())
    return "population";
  return "NONE";
}

void
    showConfig(std::ostream &out, std::string name)
{
  auto found = false;
  if (ALL_SUBSTRATE_SPECS.find(name) != ALL_SUBSTRATE_SPECS.end())
  {
    out << ALL_SUBSTRATE_SPECS[name].prettyPrint();
    found = true;
  }
  if (ALL_PROCESS_SPECS.find(name) != ALL_PROCESS_SPECS.end())
  {
    out << ALL_PROCESS_SPECS[name].prettyPrint();
    found = true;
  }
  if (ALL_POPULATION_SPECS.find(name) != ALL_POPULATION_SPECS.end())
  {
    out << ALL_POPULATION_SPECS[name].prettyPrint();
    found = true;
  }

  if (!found)
    out << "component " << name << " not found\n";

  return;
}

void
    listAllConfigs(std::ostream &out)
{
  out << "entity:\n";
  for (auto n_spec : ALL_SUBSTRATE_SPECS)
    out << "    " << n_spec.first << "\n";

  out << "environment:\n";
  for (auto n_spec : ALL_PROCESS_SPECS)
    out << "    " << n_spec.first << "\n";

  out << "population:\n";
  for (auto n_spec : ALL_POPULATION_SPECS)
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
  return;
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
    allComponentNames()
{
  auto a = allSubstrateNames();
  auto b = allProcessNames();
  auto c = allPopulationNames();
  return rv::concat(a, b, c);
}
}   // namespace config_manager

}   // namespace ded
