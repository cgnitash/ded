
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

double
    flip_bits::eval(ded::substrate &org)
{

  // feed in input - input signals are 0s or 1s only
  std::vector<double> input;
  rs::generate_n(rs::back_inserter(input), size_, []() -> double {
    return std::rand() % 2;
  });

  org.input(org_input_tag_, input);

  // give the org one tick
  org.tick();

  // get the output
  auto output_signal = org.output(org_output_tag_);
  auto output = std::get<std::vector<double>>(output_signal);
  if (output.size() != size_)
  {
    std::cout << "Error: process-flip_bits must receive an ouput of same "
                 "size as input\n";
    exit(1);
  }
  // score is the number of bit-wise matches between input and output
  return rs::inner_product(
      input, output, 0.0, std::plus<>(), [](auto a, auto b) {
        return 1 - std::abs(a - ded::utilities::Bit(b));
      });
}

ded::population
    flip_bits::evaluate(ded::population pop)
{
  pop.merge(pop.getAsVector() | ra::transform([this](auto &org) {
			  org.reset();
              org.data.setValue(flippiness_tag_, eval(org));
              return org;
            }));
  return pop;
}
