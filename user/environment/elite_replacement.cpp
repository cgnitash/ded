

#include "elite_replacement.h"

#include <algorithm>
#include <iostream>
#include <vector>

std::vector<life::entity>
elite_replacement::evaluate(const std::vector<life::entity> &pop) {

  auto temp_pop = pop;
  std::string name = "score";
  int frac = pop.size() * strength_;
  std::nth_element(std::begin(temp_pop), std::begin(temp_pop) + frac,
                   std::end(temp_pop),
                   [&name](const auto &org1, const auto &org2) {
                     return org1.data[name] > org2.data[name];
                   });

  std::vector<life::entity> new_pop;
  for (auto i = 0; i < frac; i++) {
    auto org = pop[i];
    new_pop.push_back(org);
    for (auto j = 0; j < 1 / strength_ - 1; j++) {
      auto mut_org{org};
      mut_org.mutate();
      new_pop.push_back(mut_org);
    }
  }

  new_pop.erase(std::begin(new_pop) + pop.size(), std::end(new_pop));
  if (pop.size() > new_pop.size()) {
    std::cout << "elite_replacement error  " << pop.size() << " "
              << new_pop.size();
    exit(1);
  }
  return new_pop;
}

