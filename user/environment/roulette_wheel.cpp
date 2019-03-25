

#include "roulette_wheel.h"
#include "../../core/utilities.h"

#include <algorithm>
#include <iostream>
#include <vector>

life::population
    roulette_wheel::evaluate(life::population populate)
{

  auto pop = populate.get_as_vector();

  const std::vector<double> wheel =
      pop | ranges::view::transform([this](auto org) {
        return std::get<double>(org.data.get_value(value_tag_));
      }) |
      ranges::view::partial_sum();

  // done with value_tag_
  for (auto &org : pop) org.data.clear(value_tag_);

  std::mt19937                           spinner;
  std::uniform_real_distribution<double> croupier(0, wheel.back());

  std::vector<life::entity> new_pop = ranges::view::generate_n(
      [&] {
        auto org = pop[ranges::distance(
            ranges::begin(wheel),
            ranges::lower_bound(wheel, croupier(spinner)))];
		org.mutate();
		return org;
      },
      pop.size());


  populate.merge(new_pop);
  return populate;
}
