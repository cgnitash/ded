

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

long ded::concepts::Substrate::entity_id_ = 0;

std::string ded::GLOBAL_PATH = "./";

std::map<std::string, ded::specs::SubstrateSpec>      ded::ALL_SUBSTRATE_SPECS;
std::map<std::string, ded::specs::ProcessSpec> ded::ALL_PROCESS_SPECS;
std::map<std::string, ded::specs::PopulationSpec>  ded::ALL_POPULATION_SPECS;
std::map<std::string, ded::specs::EncodingSpec>  ded::ALL_ENCODING_SPECS;

int
    main(int argc, char **argv) try
{

  // check all things that aren't being checked statically, 
  // in particular the correct publication of components
  ded::generateAllSpecs();

  std::string mode = argv[1];

  if (argc == 2 && mode == "-h")
  {
    std::cout << R"~~(
              -s                     : saves configuration files 
              -rl <file-name> <N>    : 'runs' all experiments in this file-name with N replicates (locally)
              #-rh <file-name> <N>    : 'runs' all experiments in this file-name with N replicates (msu hpc)
              -v <file-name>         : verify experiment in file-name
              -a <file-name> <trace-name>        : generate 'analysis' for experiment in file-name
              -p <component-name>... : print publication for listed component names
              -pa                    : lists all components currently loaded
              -f <bar-code> <N>      : actually runs this experiment with REP N (should NOT be called manually)
)~~";
  }
  else if (argc == 2 && mode == "-s")
  {
    std::cout << "saving configurations.cfg ... \n";
    ded::config_manager::saveAllConfigs();
  }
  else if (argc == 2 && mode == "-pa")
  {
    ded::config_manager::listAllConfigs(std::cout);
    std::cout << std::endl;
  }
  else if (argc > 2 && mode == "-p")
  {
    for (auto i{ 2 }; i < argc; i++)
      ded::config_manager::showConfig(std::cout, std::string(argv[i]));
    std::cout << std::endl;
  }
  else if (argc == 3 && mode == "-v")
  {
    auto simulations = ded::experiments::parseAllSimulations(argv[2]);
    for (auto sim : simulations)
      std::cout << sim.prettyName() << "\n";
    std::cout << "All simulations are valid - this experiment is correct\n";
  }
  else if (argc == 4 && ((mode == "-rl")))
  {
    ded::experiments::prepareSimulationsLocally(
        ded::experiments::checkSimulations(
            ded::experiments::parseAllSimulations(argv[2])),
        std::stoi(argv[3]));
	std::cout << "execute run.sh\n";
  }
  else if (argc == 4 && ((mode == "-rh")))
  {
    ded::experiments::prepareSimulationsMsuHpcc(
        ded::experiments::checkSimulations(
            ded::experiments::parseAllSimulations(argv[2])),
        std::stoi(argv[3]));
	std::cout << "execute run.sh\n";
  }
  else if (argc == 4 && ((mode == "-a")))
  {
    ded::experiments::analyseAllSimulations(argv[2], argv[3]);
	std::cout << "execute Rscript analysis.R\n";

  }
  else if (argc == 4 && mode == "-f")
  {
    auto [pop_spec, env_spec] = ded::experiments::loadSimulation(argv[2]);

    auto pop         = ded::makePopulation(pop_spec);
    auto env         = ded::makeProcess(env_spec);
    ded::GLOBAL_PATH = "./data/" + std::string{ argv[2] } + "/" + argv[3] + "/";
    std::experimental::filesystem::create_directory(ded::GLOBAL_PATH);

    std::srand(std::stoi(argv[3]));

    pop = env.evaluate(pop);
    pop.flushUnpruned();

    // should write success report to file
    std::cout << "simulation " << argv[2] << " - Replicate # " << argv[3]
              << " successfully simulated\n";
  }
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
  std::cout << "\nFATAL: this is a core issue - please file a bug report\n";
}
