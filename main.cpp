

#include "components.h"

#include <algorithm>
#include <experimental/filesystem>
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

  ded::generate_all_specs();
  // check all things that aren't being checked statically, in particular the
  // publications of components
  // ded::config_manager::check_all_configs_correct();

  //std::hash<std::string> hash_fn{};

  std::string mode = argv[1]; 

  if (argc == 2 && mode == "-h")
  {
    std::cout << R"~~(
              -s                     : saves configuration files 
              -rl <file-name> <N>    : 'runs' all experiments in this file-name with N replicates (locally)
              #-rh <file-name> <N>    : 'runs' all experiments in this file-name with N replicates (msu hpc)
              -v <file-name>         : verify experiment in file-name
              #-a <file-name>         : generate 'analysis' for experiment in file-name
              -p <component-name>... : print publication for listed component names
              -pa                    : lists all components currently loaded
              -f <exp-name> <N>      : actually runs this experiment with REP N (should NOT be called manually)
)~~";
  }
  else if (argc == 2 && mode == "-s")
  {
    std::cout << "saving configurations.cfg ... \n";
    ded::config_manager::save_all_configs();
  }
  else if (argc == 2 && mode == "-pa")
  {
    ded::config_manager::list_all_configs(std::cout);
    std::cout << std::endl;
  }
  else if (argc > 2 && mode == "-p")
  {
    for (auto i{ 2 }; i < argc; i++)
      ded::config_manager::show_config(std::cout, std::string(argv[i]));
    std::cout << std::endl;
  }
  else if (argc == 3 && mode == "-v")
  {
    auto simulations = ded::experiments::parse_all_simulations(argv[2]);
    for (auto sim : simulations)
      std::cout << sim.pretty_name() << "\n";
    std::cout << "All simulations are valid - this experiment is correct\n";
  }
  else if (argc == 4 && ((mode == "-rl")))
  {
    auto simulations = ded::experiments::parse_all_simulations(argv[2]);
    ded::experiments::prepare_simulations_locally(simulations,
                                                  std::stoi(argv[3]));
  }
  else if (argc == 4 && ((mode == "-rh")))
  {
    auto simulations = ded::experiments::parse_all_simulations(argv[2]);
    ded::experiments::prepare_simulations_msuhpc(simulations,
                                                 std::stoi(argv[3]));
  }
  else if (argc == 4 && ((mode == "-a")))
  {
    ded::experiments::analyse_all_simulations(argv[2], std::stoi(argv[3]));
  }
  else if (argc == 4 && mode == "-f")
  {
    auto [pop_spec, env_spec] = ded::experiments::load_simulation(argv[2]);

	env_spec.record_traces();

    auto pop         = ded::make_Population(pop_spec);
    auto env         = ded::make_Environment(env_spec);
    ded::global_path = "./data/" + std::string{ argv[2] } + "/" + argv[3] + "/";
    std::experimental::filesystem::create_directory(ded::global_path);

	std::srand(std::stoi(argv[3]));

    pop = env.evaluate(pop);
    pop.flush_unpruned();

	// should write success report to file
    std::cout << "simulation " << argv[2] << " - Replicate # " << argv[3]
              << " successfully simulated\n";
  }

  /*
   }  else if (argc == 4 && ((mode == "-rl") ||
                             (mode == "-rh")))
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
      if (mode == "-rl")
      {
        run_file << "for i in "
                 << ranges::accumulate(
                        exps,
                        std::string{},
                        [](auto ret, auto s) { return ret + s + " "; })
                 << " ; do for r in "
                 << ranges::accumulate(ranges::view::iota(0,
  std::stoi(argv[2])), std::string{},
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
    } else if (argc == 3 && mode == "-a")
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
                    << " with label " << label << " to analyse\n. Aborting
  ...\n"; std::exit(1);
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
    } else if (argc == 4 && mode == "-f")
    {
      auto exp_dir = std::string{ argv[3] };
      if (!std::experimental::filesystem::exists(exp_dir))
      {
        std::cout << "error: no directory " << exp_dir
                  << " found. Please DON'T modify the data/ directory
  manually\n"; std::exit(1);
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
}
catch (const ded::language::ParserError &)
{
}
catch (const ded::specs::SpecError &)
{
}
catch (...)
{
  std::cout << "\nfatal: this is a core bug - please file a bug report\n";
}
