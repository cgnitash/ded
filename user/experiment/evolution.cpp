
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
  std::vector<life::entity> pop(pop_size);
  std::generate(std::begin(pop), std::end(pop), [this]() {
    auto org = life::make_entity(org_name);
    org.configure(org_config);
    return org;
  });

  std::ofstream file("avg.csv");
  file << "avg\n";
  for (int i = 0; i < generations; i++) {

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

