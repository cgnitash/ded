
#include <experimental/filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>

#include "configuration.hpp"
#include "experiments.hpp"
#include "utilities/term_colours.hpp"
#include "utilities/utilities.hpp"

namespace ded
{
namespace experiments
{

SpecVariant
    variableToSpec(std ::string type, language::Block block)
{
  auto name = block.name_token_.expr_.substr(1);

  if (type == "process")
  {
    if (!config_manager::isProcessBlock(name))
    {
      errInvalidToken(block.name_token_,
                      "this is not an exisiting process component",
                      config_manager::allProcessNames());
      throw config_manager::ConfigError{};
    }
    return ded::specs::ProcessSpec{ block };
  }
  else if (type == "substrate")
  {
    if (!config_manager::isSubstrateBlock(name))
    {
      errInvalidToken(block.name_token_,
                      "this is not an exisiting substrate component",
                      config_manager::allSubstrateNames());
      std::cout << name + " is not a component of type " + type;
      throw config_manager::ConfigError{};
    }
    return ded::specs::SubstrateSpec{ block };
  }
  else if (type == "population")
  {
    if (!config_manager::isPopulationBlock(name))
    {
      errInvalidToken(block.name_token_,
                      "this is not an exisiting population component",
                      config_manager::allPopulationNames());
      std::cout << name + " is not a component of type " + type;
      throw config_manager::ConfigError{};
    }
    return ded::specs::PopulationSpec{ block };
  }
  else if (type == "converter")
  {
    if (!config_manager::isConverterBlock(name))
    {
      errInvalidToken(block.name_token_,
                      "this is not an exisiting converter component",
                      config_manager::allConverterNames());
      std::cout << name + " is not a component of type " + type;
      throw config_manager::ConfigError{};
    }
    return ded::specs::ConverterSpec{ block };
  }
  else if (type == "encoding")
  {
    if (!config_manager::isEncodingBlock(name))
    {
      errInvalidToken(block.name_token_,
                      "this is not an exisiting encoding component",
                      config_manager::allEncodingNames());
      std::cout << name + " is not a component of type " + type;
      throw config_manager::ConfigError{};
    }
    return ded::specs::EncodingSpec{ block };
  }
  else
  {
    std::cout << " unknown component type " + type;
    throw config_manager::ConfigError{};
  }
}

Simulation
    parseSimulation(language::Parser parser)
{

  std::map<std::string, SpecVariant> variables;

  auto parser_variables = parser.variables();
  for (auto [type_token, name_token, block] : parser_variables)
    variables[name_token.expr_] = variableToSpec(
        type_token.expr_, block);

  if (variables.find("Process") == variables.end())
  {
    std::cout << "error: " << parser.file_name()
              << " does not define a process named 'Process'\n";
    throw specs::SpecError{};
  }
  if (!std::holds_alternative<ded::specs::ProcessSpec>(variables["Process"]))
  {
    std::cout << "error: 'Process' must be of type process\n";
    throw specs::SpecError{};
  }

  auto process_spec = std::get<ded::specs::ProcessSpec>(variables["Process"]);
  process_spec.instantiateUserParameterSizes();

  if (variables.find("Population") == variables.end())
  {
    std::cout << "error: " << parser.file_name()
              << " does not define population 'Population'\n";
    throw specs::SpecError{};
  }
  if (!std::holds_alternative<ded::specs::PopulationSpec>(
          variables["Population"]))
  {
    std::cout << "error: 'Population' must be of type population\n";
    throw specs::SpecError{};
  }
  auto population_spec =
      std::get<ded::specs::PopulationSpec>(variables["Population"]);

  process_spec.bindSubstrateIO(
      population_spec.instantiateNestedSubstrateUserParameterSizes());

  process_spec.bindTags();

  process_spec.recordTraces();

  return {
    population_spec, process_spec, parser.labels(), process_spec.queryTraces()
  };
}

std::vector<language::Parser>
    expandAllTokens(language::Parser p)
{
  std::vector<language::Parser> exploded_parsers = { p };

  while (true)
  {
    std::vector<language::Parser> next_explosion;
    for (auto p : exploded_parsers)
      for (auto ep : p.varyParameter())
        next_explosion.push_back(ep);

    if (next_explosion.size() == exploded_parsers.size())
      break;
    exploded_parsers = next_explosion;
  }

  return exploded_parsers;
}

std::vector<Simulation>
    parseAllSimulations(std::string file_name)
{
  language::Lexer lexer{ file_name };

  ded::language::Parser p;
  p.updateLexer(lexer);

  auto exploded_parsers = expandAllTokens(p);

  for (auto &p : exploded_parsers)
  {
    p.parseFromLexer();
    p.resolveTrackedWords();
  }

  return exploded_parsers | rv::transform(parseSimulation) |
         rs::to<std::vector<Simulation>>;
}

std::pair<specs::PopulationSpec, specs::ProcessSpec>
    loadSimulation(std::string exp_name)
{

  auto exp_path = "./data/" + exp_name;

  std::ifstream            pop_file(exp_path + "/pop.spec");
  std::vector<std::string> ls;
  std::string              l;
  while (std::getline(pop_file, l))
    ls.push_back(l);
  ded::specs::PopulationSpec pop_spec;
  pop_spec.deserialise(ls);

  std::ifstream            env_file(exp_path + "/env.spec");
  std::vector<std::string> es;
  std::string              e;
  while (std::getline(env_file, e))
    es.push_back(e);
  ded::specs::ProcessSpec env_spec;
  env_spec.deserialise(es);

  return { pop_spec, env_spec };
}

void
    prepareSimulationsMsuHpcc(const std::vector<std::string> &exp_names,
                              int                             replicate_count)
{
  std::ofstream sb_script("./run.sb");
  sb_script << R"~(#!/bin/bash -login
#SBATCH --time=03:56:00
#SBATCH --mem=2GB
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
		
#SBATCH --array=0-)~"
            << replicate_count - 1 <<
      R"~(
cd ${SLURM_SUBMIT_DIR}
./ded -f $1 ${SLURM_ARRAY_TASK_ID} 
)~";

  std::ofstream experiment_script("./run.sh");
  experiment_script << "\nfor exp in ";
  for (auto e : exp_names)
    experiment_script << e << " ";
  experiment_script << "; do sbatch run.sb $exp ; done\n";
}

void
    prepareSimulationsLocally(const std::vector<std::string> &exp_names,
                              int                             replicate_count)
{

  std::ofstream experiment_script("./run.sh");
  experiment_script << "\nfor exp in ";
  for (auto e : exp_names)
    experiment_script << e << " ";
  experiment_script << "; do for rep in ";
  for (auto i : rv::iota(0, replicate_count))
    experiment_script << i << " ";
  experiment_script << "; do ./ded -f $exp $rep ; done ; done\n";
}

std::vector<std::string>
    checkSimulations(const std::vector<Simulation> &simulations)
{

  auto data_path = "./data/";
  if (!std::experimental::filesystem::exists(data_path))
    std::experimental::filesystem::create_directory(data_path);

  std::vector<std::string> exp_names;

  for (auto sim : simulations)
  {

    auto exp_name      = sim.barCode();
    auto exp_data_path = data_path + exp_name + "/";

    std::string prettyName = sim.prettyName();

    if (std::experimental::filesystem::exists(exp_data_path))
    {
      std::cout << "Warning: simulation " << prettyName
                << " already exists. Skipping this simulation\n";
    }
    else
    {
      std::cout << "preparing simulation....\n";
      std::experimental::filesystem::create_directory(exp_data_path);

      std::ofstream env_spec_file(exp_data_path + "env.spec");
      for (auto l : sim.process_spec.serialise(0, true))
        env_spec_file << l << "\n";
      std::ofstream pop_spec_file(exp_data_path + "pop.spec");
      for (auto l : sim.population_spec.serialise(0))
        pop_spec_file << l << "\n";

      exp_names.push_back(exp_name);
      std::cout << "simulation " << prettyName << " successfully prepared\n";
    }
  }
  if (exp_names.empty())
  {
    std::cout << "error: no experiments to simulate\n";
    throw specs::SpecError{};
  }
  return exp_names;
}

}   // namespace experiments
}   // namespace ded
