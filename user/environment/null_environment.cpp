
#include "null_environment.h"
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
null_environment::evaluate(const std::vector<life::entity> &pop) {

  life::eval_results scores;
  std::transform(std::begin(pop), std::end(pop), std::back_inserter(scores),
                 [](auto &org) {
                   std::map<std::string, std::string> m;
                   m["score"] = "0.0"; 
                   return std::make_pair(org, m);
                 });
  return scores;
}

