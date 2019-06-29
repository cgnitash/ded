

#include "components.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>


long ded::concepts::Entity::entity_id_ = 0;

std::string ded::global_path = "./";

std::map<std::string, ded::specs::EntitySpec>      ded::all_entity_specs;
std::map<std::string, ded::specs::EnvironmentSpec> ded::all_environment_specs;
std::map<std::string, ded::specs::PopulationSpec>  ded::all_population_specs;

int
    main(int argc, char **argv) try
{
  ded::global_path = "./";

  ded::generate_all_specs();
  // check all things that aren't being checked statically, in particular the
  // publications of components
  // ded::config_manager::check_all_configs_correct();

  // std::hash<std::string> hash_fn{};

  if (argc == 2 && std::string(argv[1]) == "-h")
  {
    std::cout << R"~~(
              -s                     : saves configuration files 
              #-rl <N> <file-name>    : 'runs' all experiments in this file-name with N replicates (locally)
              #-rh <N> <file-name>    : 'runs' all experiments in this file-name with N replicates (msu hpc)
              #-v <file-name>         : verify experiment in file-name
              #-a <file-name>         : generate 'analysis' for experiment in file-name
              -p <component-name>... : print publication for listed component names
              -pa                    : lists all components currently loaded
              #-f <N> <file-name>     : actually runs this experiment with REP N (should NOT be called manually)
              -r <f> <f> : pop env   	
              -d <file-name>         : parse experiment in file-name
)~~";
  } else if (argc == 2 && std::string(argv[1]) == "-s")
  {
    std::cout << "saving configurations.cfg ... \n";
    ded::config_manager::save_all_configs();
  } else if (argc == 2 && std::string(argv[1]) == "-pa")
  {
    ded::config_manager::list_all_configs(std::cout);
    std::cout << std::endl;
  } else if (argc > 2 && std::string(argv[1]) == "-p")
  {
    for (auto i{ 2 }; i < argc; i++)
      ded::config_manager::show_config(std::cout, std::string(argv[i]));
    std::cout << std::endl;
  } else if (argc > 3 && std::string(argv[1]) == "-r")
  {
    std::ifstream pop_file(argv[2]);
    if (!pop_file.is_open())
    {
      std::cout << "tch tch ... ";
      std::exit(1);
    }
    std::vector<std::string> ls;
    std::string              l;
    while (std::getline(pop_file, l)) ls.push_back(l);
    ded::specs::PopulationSpec pop_con;
    pop_con           = pop_con.parse(ls);
    auto          pop = ded::make_Population(pop_con);
    std::ifstream env_file(argv[3]);
    if (!env_file.is_open())
    {
      std::cout << "tsh tsh ... ";
      std::exit(1);
    }
    std::vector<std::string> es;
    std::string              e;
    while (std::getline(env_file, e)) es.push_back(e);
    ded::specs::EnvironmentSpec env_con;
    env_con  = env_con.parse(es);
    auto env = ded::make_Environment(env_con);

    ded::global_path += "data/";
    std::experimental::filesystem::create_directory(ded::global_path);
    pop = env.evaluate(pop);
    pop.flush_unpruned();
  } else if (argc > 2 && std::string(argv[1]) == "-d")
  {
    ded::language::Parser p;
    p.parse(argv[2]);
    //    auto vars = parse_all_parser_blocks(p);

    std::map<std::string,
             std::variant<ded::specs::EntitySpec,
                          ded::specs::EnvironmentSpec,
                          ded::specs::PopulationSpec>>
        vars;

    auto vs = p.variables();
    for (auto [name, bl] : vs)
    {
      // std::cout << name.expr_ << "#\n";
      // p.print(bl);
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
      std::cout << "error: " << argv[2]
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
      std::cout << "error: " << argv[2]
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

    auto e_dump = env_spec.dump(0);
	/*
    std::cout << (e_dump | ranges::view::intersperse("\n") |
                  ranges::action::join);
    std::cout << pop_spec.dump(0);
	*/
    auto pop = ded::make_Population(pop_spec);
    auto env = ded::make_Environment(env_spec);
    ded::global_path += "data/";
    std::experimental::filesystem::create_directory(ded::global_path);
    pop = env.evaluate(pop);
    pop.flush_unpruned();

    std::cout << "successfull!" << std::endl;
  }
  /*
  } else if (argc == 4 && ((std::string(argv[1]) == "-rl") ||
                           (std::string(argv[1]) == "-rh")))
  {
    std::string qst_path = argv[3];
    ded::global_path =
        qst_path.substr(0, qst_path.find_last_of('/') + 1) + "data/";

    if (!std::experimental::filesystem::exists(ded::global_path))
      std::experimental::filesystem::create_directory(ded::global_path);

    std::vector<std::string> exps;
    for (auto &[pop, env, label] : true_experiments(qst_path, hash_fn))
    {

      auto exp_name = std::to_string(hash_fn(pop.dump())) + "_" +
                      std::to_string(hash_fn(env.dump()));
      auto exp_path = ded::global_path + exp_name;
      if (std::experimental::filesystem::exists(exp_path))
      {
        std::cout << "Warning: experiment " << exp_name << " with label "
                  << label << " already exists. Skipping this experiment\n";
        continue;
      }
      std::experimental::filesystem::create_directory(exp_path);

      exps.push_back(exp_name);
      std::ofstream pop_file(exp_path + "/true_pop.json");
      pop_file << pop.dump(4);

      std::ofstream env_file(exp_path + "/true_env.json");
      env_file << env.dump(4);
    }
    std::ofstream run_file("run.sh");
    if (std::string(argv[1]) == "-rl")
    {
      run_file << "for i in "
               << ranges::accumulate(
                      exps,
                      std::string{},
                      [](auto ret, auto s) { return ret + s + " "; })
               << " ; do for r in "
               << ranges::accumulate(ranges::view::iota(0, std::stoi(argv[2])),
                                     std::string{},
                                     [](auto s, auto i) {
                                       return s + std::to_string(i) + " ";
                                     })
               << " ; do ./ded -f $r " << ded::global_path
               << "$i ; done  ; done";
    } else
    {   //  	if (std::string(argv[1] == "-rh")
      std::ofstream sb_file("run.sb");
      sb_file << R"~~(#!/bin/bash -login
#SBATCH --time=03:56:00
#SBATCH --mem=2GB
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
                )~~";
      sb_file << "\n#SBATCH --array=1-" << argv[2]
              << "\ncd ${SLURM_SUBMIT_DIR}\n./ded -f "
                 "${SLURM_ARRAY_TASK_ID} "
              << ded::global_path << "$1\n";
      for (auto &e : exps) run_file << "\nsbatch run.sb " << e;
    }
    std::cout << "\nGenerated script run.sh succesfully\n";
  } else if (argc == 3 && std::string(argv[1]) == "-a")
  {
    std::string qst_path = argv[2];
    ded::global_path =
        qst_path.substr(0, qst_path.find_last_of('/') + 1) + "data/";

    if (!std::experimental::filesystem::exists(ded::global_path))
    {
      std::cout << "Error: There is no data directory " << ded::global_path
                << " with experiments to analyse\n. Aborting ...\n";
      std::exit(1);
    }

    std::string exps   = "\nexps = list(";
    std::string labels = "\nlabels = c(";
    for (auto &[pop, env, label] : true_experiments(qst_path, hash_fn))
    {

      auto exp_name = std::to_string(hash_fn(pop.dump())) + "_" +
                      std::to_string(hash_fn(env.dump()));
      auto exp_path = ded::global_path + exp_name;
      if (!std::experimental::filesystem::exists(exp_path))
      {
        std::cout << "Error: There is no experiment in " << ded::global_path
                  << " with label " << label << " to analyse\n. Aborting ...\n";
        std::exit(1);
      }

      exps += "\"" + exp_path + "/\",";
      labels += "\"" + label + "\",";
    }
    exps.pop_back();
    labels.pop_back();
    std::ofstream run_file("anal.R");
    run_file << "\n" << exps << ")\n\n" << labels << ")\n";
    std::cout << "\nGenerated analysis script anal.R succesfully\nThese "
                 "experiments can be referred to by the indices 1 2 3 ... "
                 "as listed above\n";
  } else if (argc == 4 && std::string(argv[1]) == "-f")
  {
    auto exp_dir = std::string{ argv[3] };
    if (!std::experimental::filesystem::exists(exp_dir))
    {
      std::cout << "error: no directory " << exp_dir
                << " found. Please DON'T modify the data/ directory manually\n";
      std::exit(1);
    }

    ded::configuration env_con, pop_con;
    std::ifstream       pop_file(exp_dir + "/true_pop.json");
    pop_file >> pop_con;

    std::ifstream env_file(exp_dir + "/true_env.json");
    env_file >> env_con;

    ded::global_path = exp_dir + "/REP_" + argv[2] + "/";
    std::experimental::filesystem::create_directory(ded::global_path);
    std::srand(std::stoi(argv[2]));
    auto pop = ded::make_population(pop_con);

    auto env = ded::make_environment(env_con);

    env.evaluate(pop);

    std::cout << "\nExperiment " << exp_dir << "with rep:" << argv[2]
              << " run succesfully\n";

  } */
  else
  {
    std::cout << "ded: unknown command line arguments. try -h\n";
  }
} catch (const ded::language::ParserError &)
{
} catch (...)
{
  std::cout << "\nfatal: this is a core bug - please file a bug report\n";
}
