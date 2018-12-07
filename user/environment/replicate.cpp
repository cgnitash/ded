
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

life::population replicate::evaluate(life::population pop) {

  // make and configure nested environment
  auto env = life::make_environment(env_name_);
  env.configure(env_config_);

  auto vec = pop.get_as_vector();

  std::vector accumulated_scores(vec.size(), 0.0);

  util::repeat(num_, [&] {
    // evaluate the entire population
    // add scores to accumulated scores
    ranges::transform(env.evaluate(pop).get_as_vector(), accumulated_scores,
                         ranges::begin(accumulated_scores),
                         [](const auto &org, auto score) {
                           return score + double{org.data["score"]};
                         });
  });

  // assign accumulate scores to each org
  ranges::transform(vec, accumulated_scores, ranges::begin(vec),
                       [this](auto org, const auto score) {
                         org.data["score"] = score / this->num_;
                         return org;
                       });

  pop.merge(vec);
  return pop;
}

