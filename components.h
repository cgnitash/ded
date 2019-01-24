
#pragma once

#include "core/entity.h"
#include "core/environment.h"
#include "core/experiment.h"
#include "core/population.h"

#include "core/configuration.h"
#include "core/signal.h"
#include "core/utilities.h"

namespace life {
extern std::string global_path;

experiment  make_experiment(configuration);
population  make_population(configuration);
entity      make_entity(configuration);
environment make_environment(configuration);

template <typename M>
void
    generate_config(std::string                        comp_type,
                    M                                  component_maker,
                    std::initializer_list<std::string> component_list)
{

  for (auto &comp_name : component_list)
    all_configs[{ comp_type, comp_name }] =
        component_maker({ comp_name }).publish_configuration();
}

void generate_all_configs();
}   // namespace life
