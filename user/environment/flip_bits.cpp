
#include "flip_bits.h"
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
flip_bits::evaluate(const std::vector<life::entity> &pop) {

  life::eval_results scores;
  std::transform(std::begin(pop), std::end(pop), std::back_inserter(scores),
                 [this](auto &org) {
                   std::map<std::string, std::string> m;
                   life::signal s;
                   for (int i = 0; i < size_; i++)
                     s.push_back(std::rand() % 2);
                   auto o = org;
                   o.input(s);
                   o.tick();
                   auto r = o.output();
                   m["score"] = std::to_string(std::inner_product(
                       std::begin(s), std::end(s), std::begin(r), 0,
                       std::plus<>(),
                       [](long a, long b) { return std::abs(a - b); }));
				   return std::make_pair(org, m);
                 });
  return scores;
}

