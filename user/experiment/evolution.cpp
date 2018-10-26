
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

  std::ofstream file("data.csv");
  file << "avg,max\n";
  for (int i = 0; i < generations_; i++) {

    world.evaluate(pop);

    std::vector<double> scores;
    std::transform(std::begin(pop), std::end(pop), std::back_inserter(scores),
                   [](auto const& org) { return double{org.data["score"]}; });

    const auto avg =
        std::accumulate(std::begin(scores), std::end(scores), 0.0) / pop.size();

    const auto max = *std::max_element(std::begin(scores), std::end(scores));

    std::cout << "update: " << std::setw(3) << i << " avg: " << std::setw(3)
              << std::setprecision(2) << avg << "   max: " << std::setw(3)
              << std::setprecision(2) << max << std::endl;

    file << avg << "," << max << '\n';

    pop = optimiser.select(pop);
  }
  file.close();
}

