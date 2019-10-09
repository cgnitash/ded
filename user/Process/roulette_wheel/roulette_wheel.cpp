

#include "roulette_wheel.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

ded::concepts::Population
    roulette_wheel::evaluate(ded::concepts::Population populate)
{

  auto pop = populate.getAsVector();

  const std::vector<double> wheel =
      pop | rv::transform([this](auto org) {
        return std::get<double>(org.data.getValue(value_tag_));
      }) |
      rv::partial_sum();

  // done with value_tag_
  for (auto &org : pop)
    org.data.clear(value_tag_);

  std::mt19937                           spinner;
  std::uniform_real_distribution<double> croupier(0, wheel.back());

  populate.merge(rv::generate_n(
      [&] {
        auto org = pop[rs::distance(rs::begin(wheel),
                                    rs::lower_bound(wheel, croupier(spinner)))];
        org.mutate();
        return org;
      },
      pop.size()));

  populate.pruneLineage(invoke_++);
  return populate;
}
