
#include "../../core/utilities.h"
#include "visualizer.h"
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

void visualizer::run() {

  // generate the population
  auto pop = life::make_population(pop_name_);
  auto dir = Dir_ + Rep_ + "/";
  if (std::experimental::filesystem::exists(dir)) {
    std::cout << "warning: directory \"" << dir
              << "\" already contains data. rm this directory to write new "
                 "visualization results\n";
    std::exit(1);
  }
  std::experimental::filesystem::create_directory(dir);
  pop_config_["DIR"] = dir;
  pop.configure(pop_config_);

  auto world = life::make_environment(world_name_);
  world.configure(world_config_);

  pop = world.evaluate(pop);
}

