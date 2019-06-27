

#include "configuration.h"

namespace life
{
namespace config_manager
{

std::string
    type_of_block(std::string name)
{
  if (life::all_environment_specs.find(name) !=
      life::all_environment_specs.end())
    return "environment";
  if (life::all_entity_specs.find(name) != life::all_entity_specs.end())
    return "entity";
  if (life::all_population_specs.find(name) != life::all_population_specs.end())
    return "population";
  return "NONE";
}

void
    show_config(std::ostream &out, std::string name)
{
  auto found = false;
  if (life::all_entity_specs.find(name) != life::all_entity_specs.end())
  {
    out << life::all_entity_specs[name].pretty_print();
    found = true;
  }
  if (life::all_environment_specs.find(name) !=
      life::all_environment_specs.end())
  {
    out << life::all_environment_specs[name].pretty_print();
    found = true;
  }
  if (life::all_population_specs.find(name) != life::all_population_specs.end())
  {
    out << life::all_population_specs[name].pretty_print();
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
  for (auto n_spec : life::all_entity_specs)
    out << "    " << n_spec.first << "\n";

  out << "environment:\n";
  for (auto n_spec : life::all_environment_specs)
    out << "    " << n_spec.first << "\n";

  out << "population:\n";
  for (auto n_spec : life::all_population_specs)
    out << "    " << n_spec.first << "\n";

  return;
}

void
    save_all_configs()
{
  std::ofstream file("configurations.cfg");
  for (auto n_es : life::all_entity_specs)
    file << "\n" << n_es.second.pretty_print();
  for (auto n_es : life::all_environment_specs)
    file << "\n" << n_es.second.pretty_print();
  for (auto n_es : life::all_population_specs)
    file << "\n" << n_es.second.pretty_print();
  return;
}

}   // namespace config_manager
}   // namespace life
