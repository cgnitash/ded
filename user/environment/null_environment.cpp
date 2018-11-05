
#include "null_environment.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

std::vector<life::entity>
null_environment::evaluate(const std::vector<life::entity> &pop) {
  auto new_pop = pop;
  for (auto &org : new_pop)
    org.data["score"] = 0.0;
  return new_pop;
}

