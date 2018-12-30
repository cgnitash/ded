
#include "evolution_loop.h"

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

life::population evolution_loop::evaluate(life::population pop) {

  // ONLY life::experiments can do this
  life::global_path += "evolution_data/";

  // generate the population
  if (std::experimental::filesystem::exists(life::global_path)) {
    std::cout
        << "error: directory \"" << life::global_path
        << "\" already contains data. This will be overwritten. aborting..."
        << std::endl;
    exit(1);
  }
  std::experimental::filesystem::create_directory(life::global_path);
  //pop.configure(pop_config_);

  auto optimiser = life::make_environment(sel_name_);
  optimiser.configure(sel_config_);

  auto world = life::make_environment(world_name_);
  world.configure(world_config_);

  for (auto i : ranges::view::iota(0, generations_)) {

    pop = world.evaluate(pop);

    if (!(i % 25)) {
      auto stats = pop.get_stats(i);
      std::cout << "update:" << std::setw(6) << i << "   " << stats
                << std::endl;
    }

    pop.snapshot(i);

    pop = optimiser.evaluate(pop);

    pop.prune_lineage(i);
  }
  pop.flush_unpruned();

  return pop;
}

