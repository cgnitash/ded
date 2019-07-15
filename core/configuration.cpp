

#include <experimental/filesystem>

#include "configuration.h"

namespace ded
{
namespace config_manager
{

std::string
    type_of_block(std::string name)
{
  if (all_environment_specs.find(name) != all_environment_specs.end())
    return "environment";
  if (all_entity_specs.find(name) != all_entity_specs.end())
    return "entity";
  if (all_population_specs.find(name) != all_population_specs.end())
    return "population";
  return "NONE";
}

void
    show_config(std::ostream &out, std::string name)
{
  auto found = false;
  if (all_entity_specs.find(name) != all_entity_specs.end())
  {
    out << all_entity_specs[name].pretty_print();
    found = true;
  }
  if (all_environment_specs.find(name) != all_environment_specs.end())
  {
    out << all_environment_specs[name].pretty_print();
    found = true;
  }
  if (all_population_specs.find(name) != all_population_specs.end())
  {
    out << all_population_specs[name].pretty_print();
    found = true;
  }

  if (!found)
    out << "component " << name << " not found\n";

  return;
}

void
    list_all_configs(std::ostream &out)
{
  out << "entity:\n";
  for (auto n_spec : all_entity_specs)
    out << "    " << n_spec.first << "\n";

  out << "environment:\n";
  for (auto n_spec : all_environment_specs)
    out << "    " << n_spec.first << "\n";

  out << "population:\n";
  for (auto n_spec : all_population_specs)
    out << "    " << n_spec.first << "\n";

  return;
}

void
    save_all_configs()
{
  std::ofstream file("configurations.cfg");
  for (auto n_es : all_entity_specs)
    file << "\n" << n_es.second.pretty_print();
  for (auto n_es : all_environment_specs)
    file << "\n" << n_es.second.pretty_print();
  for (auto n_es : all_population_specs)
    file << "\n" << n_es.second.pretty_print();
  return;
}

}   // namespace config_manager

namespace experiments
{

std::vector<std::string>
    open_file(std::string file_name)
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
    lex(std::vector<std::string> lines)
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
        tokens.push_back(language::Token{ language::parse_token_type(m.str()),
                                          m.str(),
                                          { line_number, i - line.cbegin() } });

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

std::pair<specs::PopulationSpec, specs::EnvironmentSpec>
    parse_simulation(language::Parser p)
{

  std::map<std::string,
           std::variant<ded::specs::EntitySpec,
                        ded::specs::EnvironmentSpec,
                        ded::specs::PopulationSpec>>
      vars;

  auto parser_variables = p.variables();
  for (auto [name, bl] : parser_variables)
  {
    auto ct = ded::config_manager::type_of_block(bl.name_.substr(1));
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
              << " does not have environment E to generate\n";
    throw std::logic_error{ "" };
  }
  if (!std::holds_alternative<ded::specs::EnvironmentSpec>(vars["E"]))
  {
    std::cout << "error: E must be of type environment\n";
    throw std::logic_error{ "" };
  }

  auto env_spec = std::get<ded::specs::EnvironmentSpec>(vars["E"]);
  env_spec.instantiate_user_parameter_sizes();

  if (vars.find("P") == vars.end())
  {
    std::cout << "error: " << p.file_name()
              << " does not have population P to seed\n";
    throw std::logic_error{ "" };
  }
  if (!std::holds_alternative<ded::specs::PopulationSpec>(vars["P"]))
  {
    std::cout << "error: P must be of type population\n";
    throw std::logic_error{ "" };
  }
  auto pop_spec = std::get<ded::specs::PopulationSpec>(vars["P"]);
  auto io       = pop_spec.instantiate_nested_entity_user_parameter_sizes();

  env_spec.bind_entity_io(io);

  env_spec.bind_tags(0);

  env_spec.record_traces();

  return { pop_spec, env_spec };
}

std::vector<language::Parser>
    explode_all_tokens(language::Parser p)
{
  std::vector<language::Parser> exploded_parsers = { p };

  while (true)
  {
    std::vector<language::Parser> next_explosion =
        exploded_parsers |
        ranges::view::transform(&language::Parser::vary_parameter) |
        ranges::action::join;
    if (next_explosion.size() == exploded_parsers.size())
      break;
    exploded_parsers = next_explosion;
  }

  return exploded_parsers;
}

std::vector<std::pair<specs::PopulationSpec, specs::EnvironmentSpec>>
    parse_all_simulations(std::string file_name)
{
  auto lines  = open_file(file_name);
  auto tokens = lex(lines);

  ded::language::Parser p(language::SourceTokens{ file_name, lines, tokens });

  auto exploded_parsers = explode_all_tokens(p);

  return exploded_parsers | ranges::view::transform(parse_simulation);
}

std::pair<specs::PopulationSpec, specs::EnvironmentSpec>
    load_simulation(std::string exp_name)
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
    prepare_simulations_locally(const std::hash<std::string> & hash_fn,
                                std::string file_name,
                                int replicate_count)
{

  auto all_simulations = ded::experiments::parse_all_simulations(file_name);

  auto data_path = "./data/";
  if (!std::experimental::filesystem::exists(data_path))
    std::experimental::filesystem::create_directory(data_path);

  std::vector<std::string> exp_names;

  for (auto [pop_spec, env_spec] : all_simulations)
  {
    auto exp_name =
        std::to_string(hash_fn(pop_spec.dump(0))) + "_" +
        std::to_string(hash_fn(env_spec.dump(0, false) | ranges::action::join));
    auto exp_data_path = data_path + exp_name + "/";

    if (std::experimental::filesystem::exists(exp_data_path))
    {
      std::cout << "Warning: simulation " << exp_name
                << " already exists. Skipping this simulation\n";
    }
    else
    {
      std::cout << "preparing simulation " << exp_name << "\n";
      std::experimental::filesystem::create_directory(exp_data_path);

      std::ofstream env_spec_file(exp_data_path + "env.spec");
      for (auto l : env_spec.dump(0, true))
        env_spec_file << l << "\n";
      std::ofstream pop_spec_file(exp_data_path + "pop.spec");
      pop_spec_file << pop_spec.dump(0);

      exp_names.push_back(exp_name);
      std::cout << "simulation " << exp_name << " successfully prepared\n";
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
