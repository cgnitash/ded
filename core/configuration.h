
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

std::string typeOfBlock(std::string name);
void        show_config(std::ostream &out, std::string name);
void        listAllConfigs(std::ostream &out);
void        saveAllConfigs();

std::vector<std::string> allComponentNames();
std::vector<std::string> allEntityNames();
std::vector<std::string> allEnvironmentNames();
std::vector<std::string> allPopulationNames();
}   // namespace config_manager

}   // namespace ded
