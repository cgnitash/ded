
#include "evolution.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

void evolution::run() {
  std::vector pop(pop_size, org);

  for (int i = 0; i < generations; i++) {

    auto scores = world.evaluate(pop);

    std::cout << "avg: "
              << std::accumulate(std::begin(scores), std::end(scores), 0.0,
                                 [](double total, auto &org) {
                                   return total + std::stod(org.second["score"]);
                                 }) / scores.size()
              << std::endl;

    pop = optimiser.select(scores);
  }
}

