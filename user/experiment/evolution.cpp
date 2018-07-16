
#include"evolution.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>
#include <random>
#include <regex>

  void evolution::run() {
    auto ent = life::make_entity(entity_name);
    auto sel = life::make_selector(selector_name);
    std::vector pop(pop_size, ent);

    for (int i = 0; i < generations; i++) {

      for (auto &e : pop)
        e.update();

      std::cout << "\navg score: "
                << std::accumulate(std::begin(pop), std::end(pop), 0.f,
                                   [](auto const total, auto &value) {
                                     return total + value.score();
                                   }) /
                       pop.size();

      pop = sel.select(pop);
    }
  }


