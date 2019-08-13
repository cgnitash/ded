
#pragma once

#include "core/concepts/substrate.h"
#include "core/concepts/process.h"
#include "core/concepts/population.h"
#include "core/concepts/signal.h"

#include "core/analysis.h"
#include "core/configuration.h"
#include "core/experiments.h"

#include "core/language/parser.h"

#include "core/utilities/utilities.h"

namespace ded
{

concepts::Substrate      makeSubstrate(specs::SubstrateSpec);
concepts::Process makeProcess(specs::ProcessSpec);
concepts::Population  makePopulation(specs::PopulationSpec);

specs::SubstrateSpec      defaultSubstrateSpec(std::string);
specs::ProcessSpec defaultProcessSpec(std::string);
specs::PopulationSpec  defaultPopulationSpec(std::string);

void generateSubstrateSpec(std::initializer_list<std::string> component_list);
void
     generateProcessSpec(std::initializer_list<std::string> component_list);
void generatePopulationSpec(std::initializer_list<std::string> component_list);
void generateAllSpecs();

}   // namespace ded
