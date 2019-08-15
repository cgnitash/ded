
#include <experimental/filesystem>
#include <regex>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "utilities/term_colours.h"
#include "utilities/utilities.h"
#include "experiments.h"
#include "configuration.h"

namespace ded {
namespace experiments
{

std::vector<std::string>
    openFile(std::string file_name)
{
  std::vector<std::string> lines;
  std::ifstream            file(file_name);
  if (!file.is_open())
  {
    std::cout << "error: " << file_name << " not found\n";
    throw language::ParserError{};
  }

  std::string line;
  while (std::getline(file, line))
  {
    std::string::size_type f;
    while ((f = line.find('\t')) != std::string::npos)
      line.replace(f, 1, "    ");
    lines.push_back(line);
  }
  return lines;
}

std::vector<language::Token>
    lexTokens(std::vector<std::string> lines)
{
  std::vector<language::Token> tokens;
  for (auto [line_number, line] : rv::enumerate(lines))
  {
    line.erase(rs::find(line, '#'), line.end());

    auto i = line.cbegin();
    for (std::smatch m;
         i != line.cend() &&
         std::regex_search(i, line.cend(), m, language::Parser::valid_symbol_);
         i += m.str().length())
      if (!rs::all_of(m.str(), ::isspace))
      {
        auto type = language::parseTokenType(m.str());
        tokens.push_back(language::Token{
            type,
            m.str(),
            { line_number, i - line.cbegin() },
            type == language::TokenType::tracked_word ? m.str()
                                                      : std::string{} });
      }
    if (i != line.cend())
    {

      std::cout << "parse-error:" << line_number + 1 << ":"
                << i - line.cbegin() + 1 << "\n"
                << lines[line_number] << "\n"
                << std::string(i - line.cbegin(), ' ')
                << utilities::TermColours::red_fg << "^ unknown symbol\n"
                << utilities::TermColours::reset;
      throw language::ParserError{};
    }
  }
  return tokens;
}

Simulation
    parseSimulation(language::Parser p)
{

  std::map<std::string,
           std::variant<ded::specs::SubstrateSpec,
                        ded::specs::ProcessSpec,
                        ded::specs::PopulationSpec>>
      vars;

  auto parser_variables = p.variables();
  for (auto [name, bl] : parser_variables)
  {
    auto ct = ded::config_manager::typeOfBlock(bl.name_.substr(1));
    if (ct == "environment")
      vars[name.expr_] = ded::specs::ProcessSpec{ p, bl };
    else if (ct == "entity")
      vars[name.expr_] = ded::specs::SubstrateSpec{ p, bl };
    else if (ct == "population")
      vars[name.expr_] = ded::specs::PopulationSpec{ p, bl };
    else
    {
      std::cout << "oops: not a component!\n";
      throw std::logic_error{ "" };
    }
  }

  if (vars.find("E") == vars.end())
  {
    std::cout << "error: " << p.file_name()
              << " does not define an environment named E\n";
    throw specs::SpecError{};
  }
  if (!std::holds_alternative<ded::specs::ProcessSpec>(vars["E"]))
  {
    std::cout << "error: E must be of type environment\n";
    throw specs::SpecError{};
  }

  auto env_spec = std::get<ded::specs::ProcessSpec>(vars["E"]);
  env_spec.instantiateUserParameterSizes();

  if (vars.find("P") == vars.end())
  {
    std::cout << "error: " << p.file_name()
              << " does not define population P\n";
    throw specs::SpecError{};
  }
  if (!std::holds_alternative<ded::specs::PopulationSpec>(vars["P"]))
  {
    std::cout << "error: P must be of type population\n";
    throw specs::SpecError{};
  }
  auto pop_spec = std::get<ded::specs::PopulationSpec>(vars["P"]);
  auto io       = pop_spec.instantiateNestedSubstrateUserParameterSizes();

  env_spec.bindSubstrateIO(io);

  env_spec.bindTags(0);

  env_spec.recordTraces();

  return { pop_spec, env_spec, p.labels(), env_spec.queryTraces() };
}

std::vector<language::Parser>
    expandAllTokens(language::Parser p)
{
  std::vector<language::Parser> exploded_parsers = { p };

  while (true)
  {
    std::vector<language::Parser> next_explosion =
        exploded_parsers |
        rv::transform(&language::Parser::varyParameter) |
        ra::join;
    if (next_explosion.size() == exploded_parsers.size())
      break;
    exploded_parsers = next_explosion;
  }

  return exploded_parsers;
}

std::vector<Simulation>
    parseAllSimulations(std::string file_name)
{
  auto lines  = openFile(file_name);
  auto tokens = lexTokens(lines);

  ded::language::Parser p;
  p.updateSourceTokens(language::SourceTokens{ file_name, lines, tokens });

  auto exploded_parsers = expandAllTokens(p);

  for (auto & p : exploded_parsers) p.resolveTrackedWords();

  return exploded_parsers |
         rv::transform(parseSimulation);
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
                              int                              replicate_count)
{
  std::ofstream sb_script("./run.sb");
  sb_script << R"~(#!/bin/bash -login
#SBATCH --time=03:56:00
#SBATCH --mem=2GB
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
		
#SBATCH --array=0-)~" <<  replicate_count - 1 <<
R"~(
cd ${SLURM_SUBMIT_DIR}
//./ded -f ${SLURM_ARRAY_TASK_ID} qst/data/$1
)~";

  std::ofstream experiment_script("./run.sh");
    experiment_script << "\nfor exp in ";
    for (auto e : exp_names)
      experiment_script << e << " ";
    experiment_script << "; do sbatch run.sb $exp ; done\n";
}

void
    prepareSimulationsLocally(const std::vector<std::string> &exp_names,
                                int                            replicate_count)
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
