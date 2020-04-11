
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

ded::population
    collate::evaluate(ded::population pop)
{

  auto x1 = ded::makeprocess(x1_);

  auto x2 = ded::makeprocess(x2_);

  std::vector<double> xs;

  rs::transform(x1.evaluate(pop).getAsVector(),
                    x2.evaluate(pop).getAsVector(),
                    rs::back_inserter(xs),
                    [this](const auto &o1, const auto &o2) {
                      return (std::get<double>(o1.data.get_value(x1_tag_)) +
                              std::get<double>(o2.data.get_value(x2_tag_))) /
                             2.0;
                    });

  auto vec = pop.getAsVector();

  rs::transform(
      vec, xs, rs::begin(vec), [this](auto &org, auto &score) {
        org.data.setValue(y_tag_, score);
        return org;
      });

  pop.merge(vec);
  return pop;
}
