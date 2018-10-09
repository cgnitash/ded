
#include "max_one.h"
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

life::eval_results
max_one::evaluate(const std::vector<life::entity> &pop) {

  life::eval_results scores;
  for (auto &org : pop) {
    std::map<std::string, std::string> m;
    auto o = org;
    o.tick();
    auto v = o.output();
    m["score"] = std::to_string(std::count(std::begin(v), std::end(v), 1));
    scores.push_back(make_pair(org, m));
  }

          /*
  std::transform(std::begin(pop), std::end(pop), std::back_inserter(scores),
                 [](auto &org) {
                   std::map<std::string, std::string> m;
                   auto o = org;
				   o.tick();
                   auto v = o.output();
                   m["score"] = std::to_string(
                       std::count(std::begin(v), std::end(v), 1));
                   return std::make_pair(o, m);
                 });
				 */
  return scores;
}

