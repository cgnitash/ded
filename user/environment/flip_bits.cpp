
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

double flip_bits::eval(life::entity org) {

  life::signal s;
  for (int i = 0; i < size_; i++)
    s.push_back(std::rand() % 2);
  org.input(s);
  org.tick();
  auto r = org.output();
  return std::inner_product(std::begin(s), std::end(s), std::begin(r), 0,
                            std::plus<>(),
                            [](long a, long b) { return std::abs(a - b); });
}

life::eval_results flip_bits::evaluate(const std::vector<life::entity> &pop) {

  life::eval_results scores;
  std::transform(std::begin(pop), std::end(pop), std::back_inserter(scores),
                 [this](auto &org) {
                   std::map<std::string, std::string> m;
                   m["score"] = std::to_string(eval(org));
                   return std::make_pair(org, m);
                 });
  return scores;
}

