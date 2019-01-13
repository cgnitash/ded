
#include "collate.h"

#include <algorithm>
#include <experimental/filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

life::population collate::evaluate(life::population pop) {

  auto x1 = life::make_environment(x1_);

  auto x2 = life::make_environment(x2_);

  std::vector<double> xs;

  ranges::transform(x1.evaluate(pop).get_as_vector(),
                    x2.evaluate(pop).get_as_vector(), ranges::back_inserter(xs),
                    [this](const auto &o1, const auto &o2) {
                      return (std::get<double>(o1.data.get_value(x1_tag_)) +
                              std::get<double>(o2.data.get_value(x2_tag_))) /
                             2.0;
                    });

  auto vec = pop.get_as_vector();

  ranges::transform(vec, xs, ranges::begin(vec),
                    [this](auto &org, auto &score) {
                      org.data.set_value(y_tag_, score);
                      return org;
                    });

  pop.merge(vec);
  return pop;
}

