
#pragma once

#include "core/concepts/entity.h"
#include "core/concepts/population.h"
#include "core/concepts/environment.h"
#include "core/concepts/signal.h"

#include "core/configuration.h"

#include "core/parser/parser.h"

#include "core/utilities/utilities.h"

namespace ded {

	concepts::Entity      make_Entity(specs::EntitySpec);
 concepts::Environment make_Environment(specs::EnvironmentSpec);
 concepts::Population  make_Population(specs::PopulationSpec);

specs::EntitySpec  default_EntitySpec(std::string);
specs::EnvironmentSpec default_EnvironmentSpec(std::string);
specs::PopulationSpec default_PopulationSpec(std::string);
 
inline void
    generate_EntitySpec(std::initializer_list<std::string> component_list)
{
  for (auto comp_name : component_list)
    all_entity_specs[comp_name] = default_EntitySpec(comp_name);
}

inline void
    generate_EnvironmentSpec(std::initializer_list<std::string> component_list)
{
  for (auto comp_name : component_list)
    all_environment_specs[comp_name] = default_EnvironmentSpec(comp_name);
}

inline void
    generate_PopulationSpec(std::initializer_list<std::string> component_list)
{
  for (auto comp_name : component_list)
    all_population_specs[comp_name] = default_PopulationSpec(comp_name);
}

void generate_all_specs();

}   // namespace life
