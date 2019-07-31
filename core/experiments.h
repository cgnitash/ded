
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
      bar_code() const
  {
    std::hash<std::string> hash_fn;
    return std::to_string(
        hash_fn(population_spec.dump(0) + "_" +
                (environment_spec.dump(0, false) | ranges::action::join)));
  }

  std::string
      full_label() const
  {
    return (labels | ranges::view::transform([](auto label) {
              return label.first + " = " + label.second;
            }) |
            ranges::view::intersperse(", ") | ranges::action::join);
  }

  std::string
      pretty_name() const
  {
    return bar_code().substr(0, 4) + "... with labels [ " + full_label() + " ]";
  }
};
std::vector<std::string> open_file(std::string);

std::vector<language::Token> lex(std::vector<std::string>);

std::vector<language::Parser> expand_all_tokens(language::Parser);

Simulation parse_simulation(language::Parser);

std::vector<Simulation> parse_all_simulations(std::string);

std::pair<specs::PopulationSpec, specs::EnvironmentSpec>
    load_simulation(std::string);

void prepare_simulations_locally(const std::vector<Simulation> &, int);
void prepare_simulations_msuhpc(const std::vector<Simulation> &, int);

}   // namespace experiments
}   // namespace ded
