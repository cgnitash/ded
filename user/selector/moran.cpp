

#include "moran.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <random>

std::vector<life::entity> moran::select(life::eval_results &pop) const {

  std::string name = "score";
  std::vector<life::entity> result;

  std::nth_element(std::begin(pop), std::begin(pop) + pop.size() / 2,
                   std::end(pop), [&name](auto &org1, auto &org2) {
                     return std::stod(org1.second[name]) >
                            std::stod(org2.second[name]);
                   });

  for (auto i = 0u; i < pop.size() / 2; i++) {
    auto org = pop[i].first;
    result.push_back(org);
    org.mutate();
    result.push_back(org);
  }

  return result;
}

