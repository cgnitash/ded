
#include "visualizer.h"
#include "../../core/utilities.h"
#include <algorithm>
#include <experimental/filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

void visualizer::run()
{

  // ONLY life::experiments can do this
  life::global_path += "visualizer_data/";

  // generate the population
  auto pop = life::make_population(pop_name_);
  if (std::experimental::filesystem::exists(life::global_path))
  {
    std::cout
        << "error: directory \"" << life::global_path
        << "\" already contains data. This will be overwritten. aborting..."
        << std::endl;
    exit(1);
  }
  std::experimental::filesystem::create_directory(life::global_path);
  pop.configure(pop_config_);

  auto world = life::make_environment(world_name_);
  world.configure(world_config_);

  pop = world.evaluate(pop);
}
