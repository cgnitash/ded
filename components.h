
#pragma once

#include "core/entity.h"
#include "core/environment.h"
#include "core/population.h"
#include "core/signal.h"

#include "core/configuration.h"

#include "core/parser/parser.h"

#include "core/utilities.h"

namespace life {

entity      make_entity(entity_spec);
environment make_environment(environment_spec);
population  make_population(population_spec);

entity_spec      default_entity_spec(std::string);
environment_spec default_environment_spec(std::string);
population_spec  default_population_spec(std::string);

inline void
    generate_entity_spec(std::initializer_list<std::string> component_list)
{
  for (auto comp_name : component_list)
    all_entity_specs[comp_name] = default_entity_spec(comp_name);
}

inline void
    generate_environment_spec(std::initializer_list<std::string> component_list)
{
  for (auto comp_name : component_list)
    all_environment_specs[comp_name] = default_environment_spec(comp_name);
}

inline void
    generate_population_spec(std::initializer_list<std::string> component_list)
{
  for (auto comp_name : component_list)
    all_population_specs[comp_name] = default_population_spec(comp_name);
}

void generate_all_specs();
}   // namespace life
