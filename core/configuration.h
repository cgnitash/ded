
#pragma once

#include <map>
#include <string>
#include <vector>

#include "specs/entity_spec.h"
#include "specs/environment_spec.h"
#include "specs/population_spec.h"


namespace ded
{

extern std::string global_path;

extern std::map<std::string, specs::EntitySpec>      all_entity_specs;
extern std::map<std::string, specs::EnvironmentSpec> all_environment_specs;
extern std::map<std::string, specs::PopulationSpec>  all_population_specs;

// component_builder will provide full specializations
template <typename T>
std::string auto_class_name_as_string() = delete;

namespace config_manager
{

std::string type_of_block(std::string name);
void        show_config(std::ostream &out, std::string name);
void        list_all_configs(std::ostream &out);
void        save_all_configs();

std::vector<std::string> all_component_names();
std::vector<std::string> all_entity_names();
std::vector<std::string> all_environment_names();
std::vector<std::string> all_population_names();
}   // namespace config_manager

}   // namespace ded
