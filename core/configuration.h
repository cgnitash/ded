
#pragma once

#include <map>
#include <string>
#include <vector>

#include "specs/substrate_spec.h"
#include "specs/process_spec.h"
#include "specs/population_spec.h"


namespace ded
{

extern std::string GLOBAL_PATH;

extern std::map<std::string, specs::SubstrateSpec>      ALL_SUBSTRATE_SPECS;
extern std::map<std::string, specs::ProcessSpec> ALL_PROCESS_SPECS;
extern std::map<std::string, specs::PopulationSpec>  ALL_POPULATION_SPECS;

// component_builder will provide full specializations
template <typename T>
std::string autoClassNameAsString() = delete;

namespace config_manager
{

std::string typeOfBlock(std::string name);
void        showConfig(std::ostream &out, std::string name);
void        listAllConfigs(std::ostream &out);
void        saveAllConfigs();

std::vector<std::string> allComponentNames();
std::vector<std::string> allSubstrateNames();
std::vector<std::string> allProcessNames();
std::vector<std::string> allPopulationNames();
}   // namespace config_manager

}   // namespace ded
