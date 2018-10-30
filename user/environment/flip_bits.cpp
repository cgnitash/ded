
#include "flip_bits.h"
#include "../../core/utilities.h"
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

  // input signals are 0s or 1s only
  life::signal input;
  for (auto i{0u}; i < size_; i++)
    input.push_back(std::rand() % 2);
  org.input(input);

  // give the org one tick
  org.tick();

  auto output = org.output();
  if (output.size() != size_) {
	  std::cout << "Error: environment-flip_bits must get an ouput of same size as input\n";
	  exit(1);
  }
  // score is the number of bit-wise matches between input and output
  return std::inner_product(
      std::begin(input), std::end(input), std::begin(output), 0, std::plus<>(),
      [](long a, long b) { return std::abs(a - util::Bit(b)); });
}

void flip_bits::evaluate(std::vector<life::entity> &pop) {

  for (auto &org : pop)
    org.data["score"] = eval(org);
}

