

#include "roulette_wheel.hpp"

#include <algorithm>
#include <any>
#include <iostream>
#include <vector>

ded::concepts::Population
    roulette_wheel::evaluate(ded::concepts::Population populate)
{

  auto pop = populate.getAsVector();

  auto const wheel = pop | rv::transform([](const auto &org) {
                       return std::any_cast<double>(org.data.getValue("value"));
                     }) |
                     rv::partial_sum | rs::to<std::vector<double>>;

  std::mt19937                           spinner;
  std::uniform_real_distribution<double> croupier(0, wheel.back());

  populate.merge(
      rv::generate_n(
          [&] {
            auto org = pop[rs::distance(
                rs::begin(wheel), rs::lower_bound(wheel, croupier(spinner)))];
            org.mutate();
            return org;
          },
          pop.size()) |
      rs::to<std::vector<ded::concepts::Substrate>>);

  populate.pruneLineage(invoke_++);
  return populate;
}
