

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
  if (all_environment_specs.find(name) != all_environment_specs.end())
    return "environment";
  if (all_entity_specs.find(name) != all_entity_specs.end())
    return "entity";
  if (all_population_specs.find(name) != all_population_specs.end())
    return "population";
  return "NONE";
}

void
    show_config(std::ostream &out, std::string name)
{
  auto found = false;
  if (all_entity_specs.find(name) != all_entity_specs.end())
  {
    out << all_entity_specs[name].prettyPrint();
    found = true;
  }
  if (all_environment_specs.find(name) != all_environment_specs.end())
  {
    out << all_environment_specs[name].prettyPrint();
    found = true;
  }
  if (all_population_specs.find(name) != all_population_specs.end())
  {
    out << all_population_specs[name].prettyPrint();
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
  for (auto n_spec : all_entity_specs)
    out << "    " << n_spec.first << "\n";

  out << "environment:\n";
  for (auto n_spec : all_environment_specs)
    out << "    " << n_spec.first << "\n";

  out << "population:\n";
  for (auto n_spec : all_population_specs)
    out << "    " << n_spec.first << "\n";

  return;
}

void
    saveAllConfigs()
{
  std::ofstream file("configurations.cfg");
  for (auto n_es : all_entity_specs)
    file << "\n" << n_es.second.prettyPrint();
  for (auto n_es : all_environment_specs)
    file << "\n" << n_es.second.prettyPrint();
  for (auto n_es : all_population_specs)
    file << "\n" << n_es.second.prettyPrint();
  return;
}

std::vector<std::string>
    allEntityNames()
{
  return all_entity_specs |
         rv::transform([](auto spec) { return spec.first; });
}

std::vector<std::string>
    allEnvironmentNames()
{
  return all_environment_specs |
         rv::transform([](auto spec) { return spec.first; });
}

std::vector<std::string>
    allPopulationNames()
{
  return all_population_specs |
         rv::transform([](auto spec) { return spec.first; });
}

std::vector<std::string>
    allComponentNames()
{
  auto a = allEntityNames();
  auto b = allEnvironmentNames();
  auto c = allPopulationNames();
  return rv::concat(a, b, c);
}
}   // namespace config_manager

}   // namespace ded
