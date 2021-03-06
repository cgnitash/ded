
#pragma once

#include <string>

#include "experiments.hpp"
#include "configuration.hpp"

namespace ded
{

namespace experiments
{

void analyseAllSimulations(std::string, std::string);

std::tuple<specs::Trace, std::string, std::vector<Simulation>>
    getSingleTrace(std::vector<Simulation>, std::string);
std::pair<std::vector<int>, int> findAllStoredData(
    std::tuple<specs::Trace, std::string, std::vector<Simulation>>);
}
}
