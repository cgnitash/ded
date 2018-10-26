
#include "flip_bits.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

double flip_bits::eval(life::entity &org) {

  life::signal input;
  for (int i = 0; i < size_; i++)
    input.push_back(std::rand() % 2);
  org.input(input);
  org.tick();
  auto output = org.output();
  return std::inner_product(std::begin(input), std::end(input),
                            std::begin(output), 0, std::plus<>(),
                            [](long a, long b) { return std::abs(a - b); });
}

void flip_bits::evaluate(std::vector<life::entity> &pop) {

  for (auto &org : pop)
    org.data["score"] = eval(org);
}

