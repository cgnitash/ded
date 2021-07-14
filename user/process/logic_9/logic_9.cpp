
#include "logic_9.hpp"

#include <algorithm>
#include <experimental/filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <range/v3/view/zip_with.hpp>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

auto gen = [](int bits) {
  return rv::generate_n([] { return std::rand() % 2; }, bits) |
         rs::to<std::vector<double>>;
};

/*
auto num = [](auto rng) {
  int sum = 0;
  int pow = 1;
  for (auto i : rng)
  {
    sum += ded::utilities::Bit(i) * pow;
    pow *= 2;
  }
  return sum;
};
*/

double
    logic_9::eval(ded::concepts::Substrate &org)
{

  auto score = 0.0;
  // random starting position

  ded::utilities::repeat(tests_, [&] {
    // feed input to org; inputs are 0s and 1s only
    auto in = gen(bits_ * 2);
    org.input(args, in);
    // run the org once
    org.tick();
    // read its outputs and add to score
    auto out = std::any_cast<std::vector<double>>(org.output(result));
    std::cout << (in | rv::take(bits_));
    std::cout << (in | rv::drop(bits_));
    std::cout << rv::zip_with(
        std::logical_or{}, in | rv::take(bits_), in | rv::drop(bits_));
    for (auto o : out)
      std::cout << o << ",";
    score += rs::equal(
        rv::zip_with(std::logical_or{}, in | rv::take(bits_), in | rv::drop(bits_)),
        out);
  });
  std::cout << "\nscore: " << score << "\n";

  return score;
}

ded::concepts::Population
    logic_9::evaluate(ded::concepts::Population pop)
{
  auto vec = pop.getAsVector();
  for (auto &org : vec)
  {
    org.reset();
    org.data.setValue("logic_done", eval(org));
  }
  pop.merge(vec);
  return pop;
}
