

#include "components.hpp"
#include "core/specs/process_spec.hpp"

#include <algorithm>
#include <exception>
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

long ded::concepts::Substrate::substrate_id_ = 0;
int ded::concepts::Process::global_invocations_ = 0;

std::string ded::GLOBAL_PATH = "./";

std::map<std::string, ded::specs::SubstrateSpec>  ded::ALL_SUBSTRATE_SPECS;
std::map<std::string, ded::specs::ProcessSpec>    ded::ALL_PROCESS_SPECS;
std::map<std::string, ded::specs::PopulationSpec> ded::ALL_POPULATION_SPECS;
std::map<std::string, ded::specs::EncodingSpec>   ded::ALL_ENCODING_SPECS;
std::map<std::string, ded::specs::ConverterSpec>  ded::ALL_CONVERTER_SPECS;

struct CommandLineError
{
};

int
    main(int argc, char **argv)
try
{

  // check all things that aren't being checked statically,
  // in particular the correct publication of components
  ded::generateAllSpecs();

  if (argc == 1)
  {
    std::cout << "ded: missing command line arguments. try -h or --help\n";
    return 0;
  }

  std::string mode = argv[1];

  if (argc == 2 && (mode == "-h" || mode == "--help"))
  {
    std::cout << R"~~(
    Flags              Arguments	             Description
    -----              ---------                 -----------
    -l, --list                                   : lists all components currently loaded
    -s, --show         <component-type>:<component-name>... 
                                                 : show default configuration for each component
    -sa, --show-all                              : show default configuration for all available components
    -v, --verify       <file-name>               : verify simulations in file-name
    -rl, --run-local   <file-name> <N>           : runs all simulations in file-name with N replicates (locally)
    -rh, --run-hpc     <file-name> <N>           : runs all simulations in this file-name with N replicates (msu hpc)
    -a, --analyse      <file-name> <trace-name>  : generate analysis for simulations in file-name
    -f                 <bar-code> <N>            : run a single simulation with REP ID N (should NOT be called manually)
)~~";
  }
  else if (argc == 2 && (mode == "-l" || mode == "--list"))
  {
    ded::config_manager::listAllConfigs(std::cout);
    std::cout << std::endl;
  }
  else if (argc > 2 && (mode == "-s" || mode == "--show"))
  {
    for (auto i{ 2 }; i < argc; i++)
      ded::config_manager::showConfig(std::cout, std::string(argv[i]));
    std::cout << std::endl;
  }
  else if (argc == 2 && (mode == "-sa" || mode == "--show-all"))
  {
    ded::config_manager::saveAllConfigs();
  }
  else if (argc == 3 && (mode == "-v" || mode == "--verify"))
  {
    auto simulations = ded::experiments::parseAllSimulations(argv[2]);
    for (auto sim : simulations)
      std::cout << sim.prettyName() << "\n";
    std::cout << "All simulations are valid - this experiment is correct\n";
  }
  else if (argc == 4 && (mode == "-rl" || mode == "--run-local"))
  {
    int               replicate_count{};
    std::stringstream ss{ argv[3] };
    ss >> replicate_count;
    if (ss.fail())
      throw CommandLineError{};

    ded::experiments::prepareSimulationsLocally(
        ded::experiments::checkSimulations(
            ded::experiments::parseAllSimulations(argv[2])),
        replicate_count);
    std::cout << "execute run.sh\n";
  }
  else if (argc == 4 && (mode == "-rh" || mode == "--run-hpc"))
  {
    int               replicate_count{};
    std::stringstream ss{ argv[3] };
    ss >> replicate_count;
    if (ss.fail())
      throw CommandLineError{};

    ded::experiments::prepareSimulationsMsuHpcc(
        ded::experiments::checkSimulations(
            ded::experiments::parseAllSimulations(argv[2])),
        replicate_count);
    std::cout << "execute run.sh\n";
  }
  else if (argc == 4 && (mode == "-a" || mode == "--analyse"))
  {
    ded::experiments::analyseAllSimulations(argv[2], argv[3]);
    std::cout << "execute Rscript analysis.R\n";
  }
  else if (argc == 4 && mode == "-f")
  {
    if (!std::experimental::filesystem::exists("./data/" +
                                               std::string{ argv[2] }))
    {
      std::cout
          << "Error: invalid manually specified simulation; does not exist\n";
      std::exit(0);
    }

    auto [pop_spec, proc_spec] = ded::experiments::loadSimulation(argv[2]);

    auto pop         = ded::makePopulation(pop_spec);
    auto proc        = ded::makeProcess(proc_spec);
    ded::GLOBAL_PATH = "./data/" + std::string{ argv[2] } + "/" + argv[3] + "/";
    std::experimental::filesystem::create_directory(ded::GLOBAL_PATH);

    std::srand(std::stoi(argv[3]));

    pop = proc.evaluate(pop);
    pop.flushUnpruned();

    // should write success report to file
    std::cout << "simulation " << argv[2] << " - Replicate # " << argv[3]
              << " successfully simulated\n";
  }
  else
  {
    throw CommandLineError{};
  }

  return 0;
}
catch (const CommandLineError &)
{
  std::cout << "ded: unknown command line arguments. try -h or --help\n";
}
catch (const ded::language::ParserError &)
{
}
catch (const ded::specs::SpecError &)
{
}
catch (const ded::config_manager::ConfigError &)
{
}
catch (...)
{
  std::cout << "\nFATAL: this is a core issue - please file a bug report\n";
}
