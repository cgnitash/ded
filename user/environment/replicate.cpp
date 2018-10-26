
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
void replicate::evaluate( std::vector<life::entity> &pop) {

// TODO
// multiply scores to reduce lucky fitness evals


  auto env = life::make_environment(env_name_);
  env.configure(env_config_);
  std::vector<double> temp_scores(pop.size(), 0);
  for (auto i = 0; i < num_; i++) {
    env.evaluate(pop);
    std::transform(std::begin(pop), std::end(pop), std::begin(temp_scores),
                   std::begin(temp_scores),
                   [](const auto &org, const auto score) {
                     return score + double{org.data["score"]};
                   });
  }

  // Note that a copy of the org is being made to be visible to the caller
  std::transform(std::begin(pop), std::end(pop), std::begin(temp_scores),
                 std::begin(pop), [this](auto org, const auto score) {
                   org.data["score"] = score / num_;
                   return org;
                 });
}

