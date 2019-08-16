

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
    return "process";
  if (ALL_SUBSTRATE_SPECS.find(name) != ALL_SUBSTRATE_SPECS.end())
    return "substrate";
  if (ALL_POPULATION_SPECS.find(name) != ALL_POPULATION_SPECS.end())
    return "population";
  if (ALL_ENCODING_SPECS.find(name) != ALL_ENCODING_SPECS.end())
    return "encoding";
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
  if (ALL_ENCODING_SPECS.find(name) != ALL_ENCODING_SPECS.end())
  {
    out << ALL_ENCODING_SPECS[name].prettyPrint();
    found = true;
  }

  if (!found)
    out << "component " << name << " not found\n";

  return;
}

void
    listAllConfigs(std::ostream &out)
{
  out << "substrate:\n";
  for (auto n_spec : ALL_SUBSTRATE_SPECS)
    out << "    " << n_spec.first << "\n";

  out << "process:\n";
  for (auto n_spec : ALL_PROCESS_SPECS)
    out << "    " << n_spec.first << "\n";

  out << "population:\n";
  for (auto n_spec : ALL_POPULATION_SPECS)
    out << "    " << n_spec.first << "\n";

  out << "encoding:\n";
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
