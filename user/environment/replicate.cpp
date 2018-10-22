
#include "replicate.h"
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

life::eval_results replicate::evaluate(const std::vector<life::entity> &pop) {

// TODO
// multiply scores to reduce lucky fitness evals


 // std::cout << "here1 " << std::flush;
  auto env = life::make_environment(env_name_);
  env.configure(env_config_);
  std::vector<double> temp_scores(pop.size(), 0);
  for (auto i = 0; i < num_; i++) {
    auto s = env.evaluate(pop);
    std::transform(std::begin(s), std::end(s), std::begin(temp_scores),
                   std::begin(temp_scores),
                   [](const auto &org_s, const auto c_s) {
                     return c_s + std::stod(org_s.second.at("score"));
                   });
  }

  life::eval_results scores;
  std::transform(std::begin(pop), std::end(pop), std::begin(temp_scores),
                 std::back_inserter(scores), [this](const auto &org, const auto score) {
                   std::map<std::string, std::string> m;
                   m["score"] = std::to_string(score / num_);
                   return std::make_pair(org, m);
                 });
 // std::cout << "here 3" << std::flush;
  return scores;
}

