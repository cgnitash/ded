
#pragma once

#include <string>
#include "experiments.h"
#include "configuration.h"
namespace ded
{

namespace experiments
{

void analyse_all_simulations(std::string, std::string);

std::tuple<specs::Trace, std::string, std::vector<Simulation>>
    get_single_trace(std::vector<Simulation>, std::string);
std::pair<std::vector<int>, int> find_all_stored_data(
    std::tuple<specs::Trace, std::string, std::vector<Simulation>>);
}
}
