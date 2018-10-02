

#include "elite_replacement.h"

#include <algorithm>
#include <iostream>
#include <vector>

std::vector<life::entity>
elite_replacement::select(const std::vector<life::entity> &pop,
                          const life::eval_results &res) {

  int str = pop.size() * strength_;
  int rem = pop.size() % str;

  // get rank of current pop
  std::nth_element(
      std::begin(pop), std::begin(pop) + str, std::end(pop),
      [](auto &a, auto &b) { return a.output()[0] > b.output()[0]; });

  // new population
  std::vector<life::entity> next(pop);
  for (auto i = 1u; i < next.size() / str; i++)
    std::transform(std::begin(next), std::begin(next) + str,
                   std::begin(next) + i * str, [](auto &value) {
                     value.mutate();
                     return value;
                   });

  std::transform(std::begin(next), std::begin(next) + rem, std::end(next) - rem,
                 [](auto &value) {
                   value.mutate();
                   return value;
                 });
  return next;
}

