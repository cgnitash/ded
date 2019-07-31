

#include <experimental/filesystem>
#include <regex>
#include <range/v3/all.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "utilities/term_colours.h"
#include "configuration.h"

namespace ded
{
namespace config_manager
{

std::string
    type_of_block(std::string name)
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
    out << all_entity_specs[name].pretty_print();
    found = true;
  }
  if (all_environment_specs.find(name) != all_environment_specs.end())
  {
    out << all_environment_specs[name].pretty_print();
    found = true;
  }
  if (all_population_specs.find(name) != all_population_specs.end())
  {
    out << all_population_specs[name].pretty_print();
    found = true;
  }

  if (!found)
    out << "component " << name << " not found\n";

  return;
}

void
    list_all_configs(std::ostream &out)
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
    save_all_configs()
{
  std::ofstream file("configurations.cfg");
  for (auto n_es : all_entity_specs)
    file << "\n" << n_es.second.pretty_print();
  for (auto n_es : all_environment_specs)
    file << "\n" << n_es.second.pretty_print();
  for (auto n_es : all_population_specs)
    file << "\n" << n_es.second.pretty_print();
  return;
}

std::vector<std::string>
    all_entity_names()
{
  return all_entity_specs |
         ranges::view::transform([](auto spec) { return spec.first; });
}

std::vector<std::string>
    all_environment_names()
{
  return all_environment_specs |
         ranges::view::transform([](auto spec) { return spec.first; });
}

std::vector<std::string>
    all_population_names()
{
  return all_population_specs |
         ranges::view::transform([](auto spec) { return spec.first; });
}

std::vector<std::string>
    all_component_names()
{
  auto a = all_entity_names();
  auto b = all_environment_names();
  auto c = all_population_names();
  return ranges::view::concat(a, b, c);
}
}   // namespace config_manager

}   // namespace ded
