
#pragma once

#include <map>
#include <string>
#include <vector>

#include "specs/substrate_spec.hpp"
#include "specs/process_spec.hpp"
#include "specs/population_spec.hpp"
#include "configuration.hpp"
#include "utilities/utilities.hpp"

namespace ded
{

namespace specs
{
struct SpecError;
}

namespace experiments
{

struct Simulation
{
  specs::PopulationSpec                             population_spec;
  specs::ProcessSpec                            process_spec;
  language::Labels                                  labels;
  std::vector<std::pair<specs::Trace, std::string>> traces;

  std::string
      barCode() const
  {
    std::hash<std::string> hash_fn;
    return std::to_string(hash_fn((population_spec.serialise(0) | ra::join) + "_" +
                                  (process_spec.serialise(0, false) | ra::join)));
  }

  std::string
      fullLabel() const
  {
    return (labels | rv::transform([](auto label) {
              return label.name_ + " = " + label.value_;
            }) |
            rv::intersperse(", ") | ra::join);
  }

  std::string
      prettyName() const
  {
    return barCode() + " ... with labels [ " + fullLabel() + " ]";
  }
};

using SpecVariant = std::variant<ded::specs::SubstrateSpec,
                                 ded::specs::ProcessSpec,
                                 ded::specs::EncodingSpec,
                                 ded::specs::ConverterSpec,
                                 ded::specs::PopulationSpec>;

SpecVariant variableToSpec(std ::string type, language::Block block);

std::vector<language::Parser> expandAllTokens(language::Parser);

Simulation parseSimulation(language::Parser);

std::vector<Simulation> parseAllSimulations(std::string);

std::pair<specs::PopulationSpec, specs::ProcessSpec>
    loadSimulation(std::string);

void prepareSimulationsLocally(const std::vector<std::string> &, int);
void prepareSimulationsMsuHpcc(const std::vector<std::string> &, int);
std::vector<std::string> checkSimulations(const std::vector<Simulation> &);

}   // namespace experiments
}   // namespace ded
