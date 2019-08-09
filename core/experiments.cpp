
#include <experimental/filesystem>
#include <regex>
#include <range/v3/all.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include "utilities/term_colours.h"
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
  for (auto [line_number, line] : ranges::view::enumerate(lines))
  {
    line.erase(ranges::find(line, '#'), line.end());

    auto i = line.cbegin();
    for (std::smatch m;
         i != line.cend() &&
         std::regex_search(i, line.cend(), m, language::Parser::valid_symbol_);
         i += m.str().length())
      if (!ranges::all_of(m.str(), ::isspace))
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
           std::variant<ded::specs::EntitySpec,
                        ded::specs::EnvironmentSpec,
                        ded::specs::PopulationSpec>>
      vars;

  auto parser_variables = p.variables();
  for (auto [name, bl] : parser_variables)
  {
    auto ct = ded::config_manager::typeOfBlock(bl.name_.substr(1));
    if (ct == "environment")
      vars[name.expr_] = ded::specs::EnvironmentSpec{ p, bl };
    else if (ct == "entity")
      vars[name.expr_] = ded::specs::EntitySpec{ p, bl };
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
  if (!std::holds_alternative<ded::specs::EnvironmentSpec>(vars["E"]))
  {
    std::cout << "error: E must be of type environment\n";
    throw specs::SpecError{};
  }

  auto env_spec = std::get<ded::specs::EnvironmentSpec>(vars["E"]);
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
  auto io       = pop_spec.instantiateNestedEntityUserParameterSizes();

  env_spec.bindEntityIO(io);

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
        ranges::view::transform(&language::Parser::varyParameter) |
        ranges::action::join;
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
         ranges::view::transform(parseSimulation);
}

std::pair<specs::PopulationSpec, specs::EnvironmentSpec>
    loadSimulation(std::string exp_name)
{

  auto exp_path = "./data/" + exp_name;

  std::ifstream            pop_file(exp_path + "/pop.spec");
  std::vector<std::string> ls;
  std::string              l;
  while (std::getline(pop_file, l))
    ls.push_back(l);
  ded::specs::PopulationSpec pop_spec;
  pop_spec.parse(ls);

  std::ifstream            env_file(exp_path + "/env.spec");
  std::vector<std::string> es;
  std::string              e;
  while (std::getline(env_file, e))
    es.push_back(e);
  ded::specs::EnvironmentSpec env_spec;
  env_spec.parse(es);

  return { pop_spec, env_spec };
}

void
    prepareSimulationsMsuHpcc(const std::vector<Simulation> &, int)
{
}

void
    prepareSimulationsLocally(const std::vector<Simulation> &simulations,
                                int                            replicate_count)
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
      for (auto l : sim.environment_spec.dump(0, true))
        env_spec_file << l << "\n";
      std::ofstream pop_spec_file(exp_data_path + "pop.spec");
      pop_spec_file << sim.population_spec.dump(0);

      exp_names.push_back(exp_name);
      std::cout << "simulation " << prettyName << " successfully prepared\n";
    }
  }

  std::ofstream experiment_script("./run.sh");
  if (!exp_names.empty())
  {
    experiment_script << "\nfor exp in ";
    for (auto e : exp_names)
      experiment_script << e << " ";
    experiment_script << "; do for rep in ";
    for (auto i : ranges::view::iota(0, replicate_count))
      experiment_script << i << " ";
    experiment_script << "; do ./ded -f $exp $rep ; done ; done\n";
  }
}
}   // namespace experiments
}   // namespace ded
