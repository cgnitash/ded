
#include"cppn.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <vector>
#include <functional>
#include <numeric>
#include <algorithm>
#include <utility>
#include <map>

void cppn::mutate() {

  life::point_mutate(genome_);
  nodes.clear();
  for (auto i = 0; i < output_ + hidden_; i++) {
    Node node;
    node.activation_function = genome_[9*i] % 6;
    for (auto j = 0; j < 4; j++)
      node.in_node[genome_[9 * i + j * 2 + 1] % (i + 1)] =
          (genome_[9 * i + j * 2 + 2] % 1000) / 1000.0;
    nodes.push_back(node);
  }
}

void cppn::input(life::signal in) { ins_ = in; }

life::signal cppn::output() { return outs_; }

void cppn::tick() {

  auto results = ins_;

  for (auto &node : nodes) {
    auto const sum =
        std::accumulate(std::begin(node.in_node), std::end(node.in_node), 0.0,
                        [&results](auto const total, auto const value) {
                          return total + results[value.first] * value.second;
                        });

    results.push_back(activate(node.activation_function, sum));
  }
  std::copy( std::begin(results) + input_ + hidden_,std::end(results),std::back_inserter(outs_));
}


double cppn::activate(long c, double x) {

  auto xp = c == 0 ? std::sin(x)
                   : c == 1 ? std::cos(x)
                            : c == 2 ? std::tan(x)
                                     : c == 3 ? std::sqrt(std::abs(x))
                                              : c == 4 ? std::fmod(x, 1) : x;
  return std::clamp(xp, -2 * PI, 2 * PI);
}

// for debugging purposes

void cppn::print() {
  for (auto &node : nodes) {
    std::cout << "af: " << node.activation_function << " ws: ";
    for (auto &w : node.in_node)
      std::cout << "{" << w.first << "," << w.second << "} ";
    std::cout << std::endl;
  }
}

