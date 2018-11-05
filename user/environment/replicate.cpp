
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

std::vector<life::entity>
replicate::evaluate(const std::vector<life::entity> &pop) {

  // TODO
  // multiply scores to reduce lucky fitness evals

  // make and configure environment
  auto env = life::make_environment(env_name_);
  env.configure(env_config_);

  std::vector accumulated_scores(pop.size(), 0.0);

  util::repeat(num_, [&] {
    // evaluate the entire population
    auto ev_pop = env.evaluate(pop);
    // add scores to accumulated scores
    std::transform(
        std::begin(ev_pop), std::end(ev_pop), std::begin(accumulated_scores),
        std::begin(accumulated_scores), [](const auto &org, const auto score) {
          return score + double{org.data["score"]};
        });
  });

  // Note that a copy of the org is being made to be visible to the caller
  std::vector<life::entity> new_pop;
  std::transform(std::cbegin(pop), std::cend(pop),
                 std::begin(accumulated_scores), std::back_inserter(new_pop),
                 [this](auto org, const auto score) {
                   org.data["score"] = score / this->num_;
                   return org;
                 });

  return new_pop;
}

