
#include "evolution.h"
#include "../../core/utilities.h"
#include <algorithm>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

void evolution::run() {

  // generate the population
  auto pop = life::make_population(pop_name_);
  auto dir = Dir_  + Rep_ + "/";
  if (std::experimental::filesystem::exists(dir)) {
    std::cout << "warning: directory \"" << dir
              << "\" already contains data. This will be overwritten. Do you "
                 "want to continue? [y,n] ";
    std::string option;
    std::cin >> option;
    if (option != "y") {
      std::cout << "aborting..." << std::endl;
      exit(0);
    }
    std::experimental::filesystem::remove_all(dir);
  }
  std::experimental::filesystem::create_directory(dir);
  pop_config_["DIR"] = dir;
  pop.configure(pop_config_);

  auto optimiser = life::make_environment(sel_name_);
  optimiser.configure(sel_config_);

  auto world = life::make_environment(world_name_);
  world.configure(world_config_);



  for (auto i : ranges::view::iota(0, generations_)) {

    pop = world.evaluate(pop);

    auto stats = pop.get_stats(i);

    if (!(i % 25)) {
	  std::cout << "update: " << i << stats << std::endl;
      pop.snapshot(i);
    }


    pop = optimiser.evaluate(pop);

    pop.prune_lineage(i);
  }
  pop.flush_unpruned();
}

