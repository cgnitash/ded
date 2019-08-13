
#pragma once

#include <map>
#include <string>
#include <vector>

#include "specs/substrate_spec.h"
#include "specs/process_spec.h"
#include "specs/population_spec.h"

#include "configuration.h"
#include "utilities/utilities.h"

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
    return std::to_string(
        hash_fn(population_spec.dump(0) + "_" +
                (process_spec.dump(0, false) | ra::join)));
  }

  std::string
      fullLabel() const
  {
    return (labels | rv::transform([](auto label) {
              return label.first + " = " + label.second;
            }) |
            rv::intersperse(", ") | ra::join);
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

std::pair<specs::PopulationSpec, specs::ProcessSpec>
    loadSimulation(std::string);

void prepareSimulationsLocally(const std::vector<std::string> &, int);
void prepareSimulationsMsuHpcc(const std::vector<std::string> &, int);
std::vector<std::string> checkSimulations(const std::vector<Simulation> &);

}   // namespace experiments
}   // namespace ded
