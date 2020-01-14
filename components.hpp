
#pragma once

#include "core/concepts/substrate.hpp"
#include "core/concepts/process.hpp"
#include "core/concepts/population.hpp"
#include "core/concepts/converter.hpp"
#include "core/concepts/encoding.hpp"
#include "core/concepts/signal.hpp"

#include "core/analysis.hpp"
#include "core/configuration.hpp"
#include "core/experiments.hpp"

#include "core/language/parser.hpp"

#include "core/utilities/utilities.hpp"

namespace ded
{

concepts::Substrate      makeSubstrate(specs::SubstrateSpec);
concepts::Process makeProcess(specs::ProcessSpec);
concepts::Population  makePopulation(specs::PopulationSpec);
concepts::Converter  makeConverter(specs::ConverterSpec);

specs::SubstrateSpec      defaultSubstrateSpec(std::string);
specs::ProcessSpec defaultProcessSpec(std::string);
specs::PopulationSpec  defaultPopulationSpec(std::string);
specs::ConverterSpec  defaultConverterSpec(std::string);

void generateSubstrateSpec(std::initializer_list<std::string> component_list);
void
     generateProcessSpec(std::initializer_list<std::string> component_list);
void generatePopulationSpec(std::initializer_list<std::string> component_list);
void generateConverterSpec(std::initializer_list<std::string> component_list);
void generateAllSpecs();

}   // namespace ded
