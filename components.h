
#pragma once

#include "core/concepts/entity.h"
#include "core/concepts/environment.h"
#include "core/concepts/population.h"
#include "core/concepts/signal.h"

#include "core/analysis.h"
#include "core/configuration.h"
#include "core/experiments.h"

#include "core/language/parser.h"

#include "core/utilities/utilities.h"

namespace ded
{

concepts::Entity      make_Entity(specs::EntitySpec);
concepts::Environment make_Environment(specs::EnvironmentSpec);
concepts::Population  make_Population(specs::PopulationSpec);

specs::EntitySpec      default_EntitySpec(std::string);
specs::EnvironmentSpec default_EnvironmentSpec(std::string);
specs::PopulationSpec  default_PopulationSpec(std::string);

void generate_EntitySpec(std::initializer_list<std::string> component_list);
void
     generate_EnvironmentSpec(std::initializer_list<std::string> component_list);
void generate_PopulationSpec(std::initializer_list<std::string> component_list);
void generate_all_specs();

}   // namespace ded
