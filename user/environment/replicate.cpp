
#include "replicate.h"
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

std::vector<life::entity> replicate::evaluate(std::vector<life::entity> pop) {

  // make and configure nested environment
  auto env = life::make_environment(env_name_);
  env.configure(env_config_);

  std::vector accumulated_scores(pop.size(), 0.0);

  util::repeat(num_, [&] {
    // evaluate the entire population
    auto ev_pop = env.evaluate(pop);
    // add scores to accumulated scores
    util::rv3::transform(ev_pop, accumulated_scores,
                         util::rv3::begin(accumulated_scores),
                         [](const auto &org, auto score) {
                           return score + double{org.data["score"]};
                         });
  });

  // assign accumulate scores to each org
  util::rv3::transform(pop, accumulated_scores, util::rv3::begin(pop),
                       [this](auto org, const auto score) {
                         org.data["score"] = score / this->num_;
                         return org;
                       });

  return pop;
}

