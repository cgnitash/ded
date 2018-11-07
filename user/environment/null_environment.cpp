
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
null_environment::evaluate(std::vector<life::entity> pop) {
  for (auto &org : pop)
    org.data["score"] = 0.0;
  return pop;
}

