

#include "moran.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <random>


std::vector<life::entity> moran::select(std::vector<life::entity> &pop) const {

  if (ran_) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(pop.begin(), pop.end(), g);
  }

  for (auto it = std::begin(pop); it != std::end(pop); it += 2) {
    if (it->score() < (it + 1)->score())
      std::swap(*it, *(it + 1));
    auto x = *it;
    x.mutate();
    *(it + 1) = x;
  }
  return pop;
}

