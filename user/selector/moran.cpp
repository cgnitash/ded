

#include "moran.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <random>

std::vector<life::entity> moran::select(std::vector<life::entity>&pop) {

  std::string name = "score";
  std::vector<life::entity> result;

  std::nth_element(std::begin(pop), std::begin(pop) + pop.size() / 2,
                   std::end(pop), [&name](const auto &org1, const auto &org2) {
                     return org1.data[name] > org2.data[name];
                   });

  for (auto i = 0u; i < pop.size() / 2; i++) {
    auto org = pop[i];
    result.push_back(org);
    org.mutate();
    result.push_back(org);
  }

  return result;
}

