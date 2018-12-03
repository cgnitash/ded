
#include "evolution.h"
#include "../../core/utilities.h"
#include <algorithm>
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

  std::ofstream pop_file(Dir_ + "pop_" + Rep_ + ".csv");
  pop_file << "avg,max,update\n";

  std::ofstream lineage_file(Dir_ + "lineage_" + Rep_ + ".csv");
  lineage_file << "id,coalesced_at,encoding_size,encoding\n";

  for (auto i : util::rv3::view::iota(0, generations_)) {

    pop = world.evaluate(pop);

    auto stats =
	   	pop.get_stats();

    if (!(i % 10)) {
      std::cout << stats << std::endl;
      std::ofstream snapshot_file(Dir_ + "snapshot_" + Rep_ + "_" +
                                  std::to_string(i) + ".csv");
      snapshot_file << "id,size,encoding\n";

      pop.snapshot(snapshot_file);
    }

    for (auto &k : stats)
      pop_file << k << ",";
    pop_file << i << std::endl;

    pop = optimiser.evaluate(pop);

    for (auto &org : pop.prune_lineage())
      lineage_file << org.get_id() << "," << i << ","
                   << org.get_encoding().size() << "," << org.get_encoding()
                   << std::endl;
  }
}

