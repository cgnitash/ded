

#include "null_selector.h"

#include <algorithm>
#include <iostream>
#include <vector>

std::vector<life::entity> null_selector::select(life::eval_results &pop) const {
  std::vector<life::entity> res;
  std::transform(std::begin(pop), std::end(pop), std::back_inserter(res),
                 [](auto &value) { return value.first; });
  return res;
}

