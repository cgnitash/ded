

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

void
    list_all_configs()
{

  std::map<std::string, std::vector<std::string>> cons;
  for (auto &c : life::all_configs)
    cons[c.first.first].push_back(c.first.second);
  for (auto &type : { "entity", "environment", "population" })
  {
    std::cout << type << "\n";
    for (auto &name : cons[type]) std::cout << "    " << name << "\n";
  }
}

std::vector<std::tuple<life::configuration, life::configuration, std::string>>
    true_experiments(std::string file_name, std::hash<std::string> hash_fn)
{

  std::vector<std::tuple<life::configuration, life::configuration, std::string>>
      exps;

  for (auto [pop_con_s, env_con_s, label] : qst_parser().parse_qst(file_name))
  {

    // why this circumlocution?
    std::stringstream es, ps;
    ps << pop_con_s;
    es << env_con_s;
    life::configuration pop_con;
    life::configuration env_con;
    ps >> pop_con;
    es >> env_con;
    auto true_pop = life::configuration::array(
        { pop_con[0],
          life::config_manager::true_any_object({ "population", pop_con[0] },
                                                pop_con[1]) });

    // assume population is linear *****
    auto true_env = life::configuration::array(
        { env_con[0],
          life::config_manager::true_environment_object(
              { "environment", env_con[0] },
              env_con[1],
              true_pop[1]["parameters"]["entity"]) });

    exps.push_back(std::make_tuple(true_pop, true_env, label));
    auto exp_name = std::to_string(hash_fn(true_pop.dump())) + "_" +
                    std::to_string(hash_fn(true_env.dump()));
    std::cout << "Verified experiment " << exp_name << " with label \"" << label
              << "\"" << std::endl;
  }
  return exps;
}

long life::entity::entity_id_ = 0;

std::string life::global_path = "./";

std::map<life::ModuleInstancePair, life::configuration> life::all_configs;

int
    main(int argc, char **argv)
{
  // TODO use an actual command-line library :P
  //

  life::generate_all_configs();
  // check all things that aren't being checked statically, in particular the
  // publications of components
  life::config_manager::check_all_configs_correct();

  std::hash<std::string> hash_fn{};

  if (argc == 2 && std::string(argv[1]) == "-h")
  {
    std::cout << R"~~(
              -s                     : saves configuration files 
              -rl <N> <file-name>    : 'runs' all experiments in this file-name with N replicates (locally)
              -rh <N> <file-name>    : 'runs' all experiments in this file-name with N replicates (msu hpc)
              -v <file-name>         : verify experiment in file-name
              -a <file-name>         : generate 'analysis' for experiment in file-name
              -p <component-name>... : print publication for listed component names
              -pa                    : lists all components currently loaded
              -f <N> <file-name>     : actually runs this experiment with REP N (should NOT be called manually)
)~~";
  } else if (argc == 2 && std::string(argv[1]) == "-s")
  {
    std::cout << "saving configurations.cfg ... \n";
    life::config_manager::save_configs();
  } else if (argc == 2 && std::string(argv[1]) == "-pa")
  {
    list_all_configs();
    std::cout << std::endl;
  } else if (argc > 2 && std::string(argv[1]) == "-p")
  {
    for (auto i{ 2 }; i < argc; i++)
      life::config_manager::show_config(std::string(argv[i]));
    std::cout << std::endl;
  } else if (argc == 3 && std::string(argv[1]) == "-v")
  {
    true_experiments(argv[2], hash_fn);
    std::cout << "\nVerified all experiments succesfully\n";

  } else if (argc == 4 && ((std::string(argv[1]) == "-rl") ||
                           (std::string(argv[1]) == "-rh")))
  {
    std::string qst_path = argv[3];
    life::global_path =
        qst_path.substr(0, qst_path.find_last_of('/') + 1) + "data/";

    if (!std::experimental::filesystem::exists(life::global_path))
      std::experimental::filesystem::create_directory(life::global_path);

    std::vector<std::string> exps;
    for (auto &[pop, env, label] : true_experiments(qst_path, hash_fn))
    {

      auto exp_name = std::to_string(hash_fn(pop.dump())) + "_" +
                      std::to_string(hash_fn(env.dump()));
      auto exp_path = life::global_path + exp_name;
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
               << " ; do ./ded -f $r " << life::global_path
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
              << life::global_path << "$1\n";
      for (auto &e : exps) run_file << "\nsbatch run.sb " << e;
    }
    std::cout << "\nGenerated script run.sh succesfully\n";
  } else if (argc == 3 && std::string(argv[1]) == "-a")
  {
    std::string qst_path = argv[2];
    life::global_path =
        qst_path.substr(0, qst_path.find_last_of('/') + 1) + "data/";

    if (!std::experimental::filesystem::exists(life::global_path))
    {
      std::cout << "Error: There is no data directory " << life::global_path
                << " with experiments to analyse\n. Aborting ...\n";
      std::exit(1);
    }

    std::string exps   = "\nexps = list(";
    std::string labels = "\nlabels = c(";
    for (auto &[pop, env, label] : true_experiments(qst_path, hash_fn))
    {

      auto exp_name = std::to_string(hash_fn(pop.dump())) + "_" +
                      std::to_string(hash_fn(env.dump()));
      auto exp_path = life::global_path + exp_name;
      if (!std::experimental::filesystem::exists(exp_path))
      {
        std::cout << "Error: There is no experiment in " << life::global_path
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

    life::configuration env_con, pop_con;
    std::ifstream       pop_file(exp_dir + "/true_pop.json");
    pop_file >> pop_con;

    std::ifstream env_file(exp_dir + "/true_env.json");
    env_file >> env_con;

    life::global_path = exp_dir + "/REP_" + argv[2] + "/";
    std::experimental::filesystem::create_directory(life::global_path);
    std::srand(std::stoi(argv[2]));
    auto pop = life::make_population(pop_con);

    auto env = life::make_environment(env_con);

    env.evaluate(pop);

    std::cout << "\nExperiment " << exp_dir << "with rep:" << argv[2]
              << " run succesfully\n";

  } else
  {
    std::cout << "ded: unknown command line arguments. try -h\n";
  }
}
