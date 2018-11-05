
#include "max_one.h"
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

std::vector<life::entity>
max_one::evaluate(const std::vector<life::entity> &pop) {

  auto new_pop = pop;
  // no inputs
  for (auto &org : new_pop) {
    // run single tick
    org.tick();
    // output size can be determined by entity
    auto out = org.output();

    // score is number of outputs that evaluate to Bit() == 1
    org.data["score"] =
        std::count_if(std::begin(out), std::end(out),
                      [](const auto i) { return util::Bit(i); });
  }

  return new_pop;
}

