
#include "evolution.h"
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
  std::vector<life::entity> pop;
  for (auto i = 0; i < pop_size_; i++) {
    auto org = life::make_entity(org_name_);
    org.configure(org_config_);
    pop.push_back(org);
  }
  auto optimiser = life::make_selector(sel_name_);
  optimiser.configure(sel_config_);
  auto world = life::make_environment(world_name_);
  world.configure(world_config_);

  std::ofstream file("avg.csv");
  file << "avg\n";
  for (int i = 0; i < generations_; i++) {

    auto scores = world.evaluate(pop);

    const auto avg =
        std::accumulate(std::begin(scores), std::end(scores), 0.0,
                        [](double total, auto &org) {
                          return total + std::stod(org.second["score"]);
                        }) /
        scores.size();

    std::cout << "avg: " << avg << std::endl;
    file << avg << '\n';

    pop = optimiser.select(scores);
  }
}

