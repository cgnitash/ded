
#include "twoDio.h"

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

ded::population
    twoDio::evaluate(ded::population pop)
{

  for (auto &org : pop.getAsVector()) {
    std::ofstream pop_stats_file{ ded::global_path + "pattern_" +
                                  std::to_string(org.getID()) + ".csv" };
    for (auto i{ -2 * ded::utilities::PI }; i < 2 * ded::utilities::PI; i += .1)
      for (auto j{ -2 * ded::utilities::PI }; j < 2 * ded::utilities::PI; j += .1)
      {
        org.input(org_input_tag_, std::vector{i , j });
		org.tick();
        auto out = std::get<std::vector<double>>(org.output(org_output_tag_));
        pop_stats_file << i << "," << j ;
        for (auto o : out) pop_stats_file << "," << o;
        pop_stats_file<< "\n";
      }
  }

  return pop;
}
