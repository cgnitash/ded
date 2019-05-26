
#pragma once

#include "core/entity.h"
#include "core/environment.h"
#include "core/population.h"
#include "core/signal.h"

#include "core/configuration.h"

#include "core/utilities.h"

namespace life {

entity      make_entity(entity_spec);
environment make_environment(environment_spec);
population  make_population(population_spec);

void
    generate_entity_spec(std::initializer_list<std::string> component_list)
{
  for (auto comp_name : component_list)
    all_entity_specs[ comp_name ] =
        make_entity(entity_spec{ comp_name }).publish_configuration();
}

void
    generate_environment_spec(std::initializer_list<std::string> component_list)
{
  for (auto comp_name : component_list)
    all_environment_specs[ comp_name ] =
        make_environment(environment_spec{ comp_name }).publish_configuration();
}

void
    generate_population_spec(std::initializer_list<std::string> component_list)
{
  for (auto comp_name : component_list)
    all_population_specs[ comp_name ] =
        make_population(population_spec{ comp_name }).publish_configuration();
}

void generate_all_configs();
}   // namespace life
