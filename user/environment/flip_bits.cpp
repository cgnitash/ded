
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

	//TODO fix some wierd bug here

  // feed in input - input signals are 0s or 1s only
  auto input = util::rv3::view::generate([] () -> double  { return std::rand() % 2; }) |
               util::rv3::view::take(size_);
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
// auto in = input; 
 std::cout << input << "\n";
 //auto out = output; 
 for (auto i: output) std::cout << i << " ";
  auto x = util::rv3::inner_product(
      input, output, 0.0, std::plus<>(),
      [](auto a, auto b) { return 1 - std::abs(a - util::Bit(b)); });
  std::cout << x << " ";
  std::cin>>x;
  return x;
}

std::vector<life::entity> flip_bits::evaluate(std::vector<life::entity> pop) {
  for (auto &org : pop)
    org.data["score"] = eval(org);
  return pop;
}

