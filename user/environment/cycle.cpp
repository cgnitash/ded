
#include "cycle.h"

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

life::population cycle::evaluate(life::population pop) {

  life::global_path += "cycle/";

  if (std::experimental::filesystem::exists(life::global_path)) {
    std::cout
        << "error: directory \"" << life::global_path
        << "\" already contains data. This will be overwritten. aborting..."
        << std::endl;
    exit(1);
  }
  std::experimental::filesystem::create_directory(life::global_path);

  auto world = life::make_environment(world_);

  for (auto i : ranges::view::iota(0, count_)) {

    pop = world.evaluate(pop);

    if (!snap_freq_ && !(i % snap_freq_))
      pop.snapshot(i);

	// this should not be necessary to say
    pop.prune_lineage(i);
  }
  // this should not be necessary to say
  pop.flush_unpruned();

  // yuck
  life::global_path =
      life::global_path.substr(0, life::global_path.length() - 6);

  return pop;
}

