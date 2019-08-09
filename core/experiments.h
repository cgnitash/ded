
#pragma once

#include <map>
#include <string>
#include <vector>

#include "specs/entity_spec.h"
#include "specs/environment_spec.h"
#include "specs/population_spec.h"

#include "configuration.h"

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
  specs::EnvironmentSpec                            environment_spec;
  language::Labels                                  labels;
  std::vector<std::pair<specs::Trace, std::string>> traces;

  std::string
      barCode() const
  {
    std::hash<std::string> hash_fn;
    return std::to_string(
        hash_fn(population_spec.dump(0) + "_" +
                (environment_spec.dump(0, false) | ranges::action::join)));
  }

  std::string
      fullLabel() const
  {
    return (labels | ranges::view::transform([](auto label) {
              return label.first + " = " + label.second;
            }) |
            ranges::view::intersperse(", ") | ranges::action::join);
  }

  std::string
      prettyName() const
  {
    return barCode().substr(0, 4) + "... with labels [ " + fullLabel() + " ]";
  }
};
std::vector<std::string> openFile(std::string);

std::vector<language::Token> lexTokens(std::vector<std::string>);

std::vector<language::Parser> expandAllTokens(language::Parser);

Simulation parseSimulation(language::Parser);

std::vector<Simulation> parseAllSimulations(std::string);

std::pair<specs::PopulationSpec, specs::EnvironmentSpec>
    loadSimulation(std::string);

void prepareSimulationsLocally(const std::vector<Simulation> &, int);
void prepareSimulationsMsuHpcc(const std::vector<Simulation> &, int);

}   // namespace experiments
}   // namespace ded
