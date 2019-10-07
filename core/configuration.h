
#pragma once

#include <map>
#include <string>
#include <vector>

#include "specs/encoding_spec.h"
#include "specs/population_spec.h"
#include "specs/process_spec.h"
#include "specs/substrate_spec.h"

namespace ded
{

extern std::string GLOBAL_PATH;

extern std::map<std::string, specs::SubstrateSpec>  ALL_SUBSTRATE_SPECS;
extern std::map<std::string, specs::ProcessSpec>    ALL_PROCESS_SPECS;
extern std::map<std::string, specs::PopulationSpec> ALL_POPULATION_SPECS;
extern std::map<std::string, specs::EncodingSpec>   ALL_ENCODING_SPECS;

// component_builder will provide full specializations
template <typename T>
std::string autoClassNameAsString() = delete;

namespace config_manager
{

void        showConfig(std::ostream &out, std::string name);
void        listAllConfigs(std::ostream &out);
void        saveAllConfigs();

enum class SpecType
{
	encoding, substrate, process, population, UNKNOWN
};

SpecType typeOfBlock(std::string name);

bool        isSubstrateBlock(std::string name);
bool        isEncodingBlock(std::string name);
bool        isProcessBlock(std::string name);
bool        isPopulationBlock(std::string name);

std::vector<std::string> allComponentNames();
std::vector<std::string> allSubstrateNames();
std::vector<std::string> allProcessNames();
std::vector<std::string> allPopulationNames();
std::vector<std::string> allEncodingNames();
}   // namespace config_manager

}   // namespace ded
