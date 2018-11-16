
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

life::population max_one::evaluate(life::population p) {

	auto pop = p.get_as_vector();
  p.clear();
  for (auto &org : pop) {
    // no inputs
  
    // run single tick
    org.tick();

    // score is number of outputs that evaluate to Bit() == 1
    org.data["score"] =
        util::rv3::count_if(org.output(), [](auto i) { return util::Bit(i); });
  }

  p.merge(pop);
  return p;
}

