
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
  pop.configure(pop_config_);

  auto optimiser = life::make_environment(sel_name_);
  optimiser.configure(sel_config_);

  auto world = life::make_environment(world_name_);
  world.configure(world_config_);

  auto dir = Dir_  + Rep_ + "/"; 
  std::experimental::filesystem::create_directory(dir);

  std::ofstream pop_file(dir + "pop.csv");
  pop_file << "avg,max,update\n";

  std::ofstream lineage_organisms_file(dir + "lineage_organisms.csv");
  lineage_organisms_file << "id,recorded_at,encoding_size,encoding\n";

  std::ofstream lineage_file(dir + "lineage"+ ".csv");
  lineage_file << "id,recorded_at,on_lod,ancestor_id\n";

  for (auto i : ranges::view::iota(0, generations_)) {

    pop = world.evaluate(pop);

    pop.get_stats(pop_file, i);

    if (!(i % 25)) {
      pop.get_stats(std::cout, i);
      std::ofstream snapshot_file(dir+ "snapshot_" +
                                  std::to_string(i) + ".csv");

      pop.snapshot(snapshot_file,i);
    }


    pop = optimiser.evaluate(pop);

    pop.prune_lineage(lineage_file,lineage_organisms_file,i);
  }
}

