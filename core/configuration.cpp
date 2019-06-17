
#include "configuration.h"
namespace life {
std::string global_path = "./";
std::map<std::string, entity_spec>      all_entity_specs{};
std::map<std::string, environment_spec> all_environment_specs{};
std::map<std::string, population_spec>  all_population_specs{};
}
