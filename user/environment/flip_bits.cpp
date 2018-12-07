
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

  // feed in input - input signals are 0s or 1s only
  life::signal input;
  ranges::generate_n(ranges::back_inserter(input), size_,
                        []() -> double { return std::rand() % 2; });

  org.input(input);

  // give the org one tick
  org.tick();

  // get the output
  auto output = org.output();
  if (output.size() != size_) {
    std::cout << "Error: environment-flip_bits must receive an ouput of same "
                 "size as input\n";
    exit(1);
  }
  // score is the number of bit-wise matches between input and output
  return ranges::inner_product(
      input, output, 0.0, std::plus<>(),
      [](auto a, auto b) { return 1 - std::abs(a - util::Bit(b)); });

}

life::population flip_bits::evaluate(life::population pop) {
  pop.merge(pop.get_as_vector() |
            ranges::action::transform([this](auto &org) {
              org.data["score"] = eval(org);
              return org;
            }));
  return pop;
}

