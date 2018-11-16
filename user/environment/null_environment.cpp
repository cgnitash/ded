
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

life::population null_environment::evaluate(life::population pop) {
  auto p = pop.get_as_vector();
  pop.clear();
  for (auto &org : p)
    org.data["score"] = 0.0;
  pop.merge(p);
  return pop;
}

