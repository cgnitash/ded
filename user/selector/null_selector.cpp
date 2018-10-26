

#include "null_selector.h"

#include <algorithm>
#include <iostream>
#include <vector>

std::vector<life::entity>
null_selector::select(std::vector<life::entity> &pop) {

  std::vector<life::entity> res;
  std::copy(std::begin(pop), std::end(pop),
            std::begin(res));
  return res;
}

