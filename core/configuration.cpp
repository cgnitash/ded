

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

std::vector<std::string>
    all_entity_names()
{
  return all_entity_specs |
         ranges::view::transform([](auto spec) { return spec.first; });
}

std::vector<std::string>
    all_environment_names()
{
  return all_environment_specs |
         ranges::view::transform([](auto spec) { return spec.first; });
}

std::vector<std::string>
    all_population_names()
{
  return all_population_specs |
         ranges::view::transform([](auto spec) { return spec.first; });
}

std::vector<std::string>
    all_component_names()
{
      auto a = all_entity_names();
      auto b = all_environment_names();
      auto c = all_population_names();
  return ranges::view::concat(a,b,c);
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

std::string
    Simulation::bar_code() const
{
std::hash<std::string>  hash_fn;
  return std::to_string(
      hash_fn(population_spec.dump(0) + "_" +
              (environment_spec.dump(0, false) | ranges::action::join)));
}

std::string
    Simulation::full_label()const
{
  return (labels | ranges::view::transform([](auto label) {
            return label.first + " = " + label.second;
          }) |
          ranges::view::intersperse(", ") | ranges::action::join);
}

std::string
    Simulation::pretty_name() const
{
  return bar_code().substr(0, 4) + "... with labels [ " + full_label() + " ]";
}

Simulation
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
              << " does not define an environment named E\n";
    throw specs::SpecError{};
  }
  if (!std::holds_alternative<ded::specs::EnvironmentSpec>(vars["E"]))
  {
    std::cout << "error: E must be of type environment\n";
    throw specs::SpecError{};
  }

  auto env_spec = std::get<ded::specs::EnvironmentSpec>(vars["E"]);
  env_spec.instantiate_user_parameter_sizes();

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
  auto io       = pop_spec.instantiate_nested_entity_user_parameter_sizes();

  env_spec.bind_entity_io(io);

  //int start_tag_count = 0;
  env_spec.bind_tags(0);

  //env_spec.record_traces();

  return { pop_spec, env_spec , p.labels(), env_spec.record_traces()};
}

std::vector<language::Parser>
    expand_all_tokens(language::Parser p)
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

std::vector<Simulation>
    parse_all_simulations(std::string file_name)
{
  auto lines  = open_file(file_name);
  auto tokens = lex(lines);

  ded::language::Parser p;
  p.update_source_tokens(language::SourceTokens{ file_name, lines, tokens });

  auto exploded_parsers = expand_all_tokens(p);

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
    prepare_simulations_msuhpc(const std::vector<Simulation> &, int)
{
}

void
    analyse_all_simulations(std::string file_name, int replicate_count)
{

  auto simulations = ded::experiments::parse_all_simulations(file_name);
  for (auto sim : simulations)
  {
	  if (sim.traces.size() != 1) {
		  std::cout << "error: no analysis available for multiple traces\n";
		  return;
	  }
  }

  auto trace = *(simulations.begin()->traces.begin());
  auto trace_name = trace.second + trace.first.signal_.user_name() + "_";
  std::vector<int> max_invocation_counts;
  for (auto sim : simulations)
  {
    auto bar_code = sim.bar_code();
    for (auto rep : ranges::view::iota(0, replicate_count))
    {
      auto rep_path =
          "data/" + bar_code + "/" + std::to_string(rep) + "/" + trace_name;
      std::cout << rep_path << "\n";
      int invocation = trace.first.frequency_;
      while (std::experimental::filesystem::exists(
          rep_path + std::to_string(invocation) + ".csv"))
        invocation += trace.first.frequency_;
      max_invocation_counts.push_back(invocation - trace.first.frequency_);
      std::cout << invocation << "\n";
    }
  }
  
  auto [low, high] = ranges::minmax_element(max_invocation_counts);
  if (*low != *high)
  {
    std::cout << "error: misalignment of max trace invocation counts\n";
    return;
  }

  std::ofstream analysis_file("analysis.R");
  analysis_file
      << "\n#!/usr/bin/env Rscript\nsource(\"dedli/plot.R\")\nexps=list("
      << (simulations | ranges::view::transform([](auto sim) {
            return "\"data/" + sim.bar_code() + "/\"";
          }) |
          ranges::view::intersperse(",") | ranges::action::join)
      << ")\nlabels=c("
      << (simulations | ranges::view::transform([](auto sim) {
            return "\"" + sim.full_label() + "\"";
          }) |
          ranges::view::intersperse(",") | ranges::action::join)
      << ")\ndata=un_reported_data(exps,\"/" + trace.second << "\",\""
      << trace.first.signal_.user_name() << "\",0:" << replicate_count - 1
      << ",seq(" << trace.first.frequency_ << "," << *low << ","
      << trace.first.frequency_
      << "))\nall_avg = "
         "compute_all(avg,exps,labels,data)\npdf(\"result.pdf\")"
         "\ncluster_plots(all_avg,\"avg\",labels,1:"
      << simulations.size() << ",palette(rainbow(" << simulations.size() + 1
      << ")),\"" << file_name << " Replicates " << replicate_count
      << "\")\nfinal_fitness_plots(all_avg,\"Final avg\",labels)\nfor (i in 1:"
      << simulations.size()
      << ")\n{\n "
         "plot(cluster_plots(all_avg,\"avg\",labels,i:i,palette(rainbow(2)),"
         "labels[i]))\n}\ndev.off()\n";
}

void
    prepare_simulations_locally(const std::vector<Simulation>& simulations,
                                int replicate_count)
{

  auto data_path = "./data/";
  if (!std::experimental::filesystem::exists(data_path))
    std::experimental::filesystem::create_directory(data_path);

  std::vector<std::string> exp_names;

  for (auto sim : simulations)
  {

	  /*
	auto traces = sim.environment_spec.record_traces();
	for (auto ts : traces)
          std::cout << "trace of " << ts.first.signal_.user_name() << " at freq "
                    << ts.first.frequency_ << " located at " << ts.second
                    << "\n";
	*/
	auto exp_name = sim.bar_code();
    auto exp_data_path = data_path + exp_name + "/";

    std::string pretty_name = sim.pretty_name();

    if (std::experimental::filesystem::exists(exp_data_path))
    {
      std::cout << "Warning: simulation " << pretty_name
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
      std::cout << "simulation " << pretty_name 
                << " successfully prepared\n";
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
