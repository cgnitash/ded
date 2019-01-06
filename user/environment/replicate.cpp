
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

life::population replicate::evaluate(life::population pop) {

  // make and configure nested environment
  auto env = life::make_environment(env_);

  std::vector accumulated_scores(pop.size(), 0.0);

  util::repeat(num_, [&] {
    // evaluate the entire population
    // add scores to accumulated scores
    ranges::transform(
        env.evaluate(pop).get_as_vector(), accumulated_scores,
        ranges::begin(accumulated_scores), [this](const auto &org, auto score) {
          return score + std::get<double>(org.data.get_value(used_x_tag_));
        });
  });

  auto vec = pop.get_as_vector();

  // assign accumulate scores to each org
  ranges::transform(vec, accumulated_scores, ranges::begin(vec),
                       [this](auto org, const auto score) {
                         org.data.set_value(fx_tag_, score / this->num_);
                         return org;
                       });

  pop.merge(vec);
  return pop;
}

