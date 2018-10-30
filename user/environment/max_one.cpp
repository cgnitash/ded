
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
#include <vector>
#include <utility>

void max_one::evaluate(std::vector<life::entity> &pop) {

  // no inputs
  for (auto &org : pop) {
    // run single tick
    org.tick();
	// output size can be determined by entity
    auto out = org.output();

    // score is number of outputs that evaluate to Bit() == 1
    org.data["score"] =
        std::count_if(std::begin(out), std::end(out),
                      [](const auto i) { return util::Bit(i); });
  }
}

