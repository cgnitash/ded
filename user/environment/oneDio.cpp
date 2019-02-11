
#include "oneDio.h"

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

life::population
    oneDio::evaluate(life::population pop)
{

  for (auto &org : pop.get_as_vector()) {
    std::ofstream pop_stats_file{ life::global_path + "pattern_" +
                                  std::to_string(org.get_id()) + ".csv" };
    for (auto i{ -2 * util::PI }; i < 2 * util::PI; i += .1) {
      org.input(org_input_tag_, std::vector{ i });
      org.tick();
      auto out = std::get<std::vector<double>>(org.output(org_output_tag_));
      pop_stats_file << i << "," << out[0] << "\n";
          }
  }

  return pop;
}
