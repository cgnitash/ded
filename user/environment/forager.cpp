
#include "forager.h"
#include "../../core/utilities.h"
#include "../../core/range-v3/all.hpp"

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


void forager::replace_resource_() {
  while (true) {
    auto p = location{std::rand() % grid_size_, std::rand() % grid_size_};
    auto inserted = resources_.insert(p);
    if (inserted.second)
      break;
  }
}

void forager::initialize_resource_() {

  for (size_t i : ranges::view::iota(0, grid_size_))
    for (size_t j : ranges::view::iota(0, grid_size_))
      if ((std::rand() % 100) / 100.0 < density_)
        resources_.insert(location{i, j});

}

void forager::refresh_signals() {

  signal_strength_.clear();
  auto surface = resources_;

  util::repeat(sensor_range_, [&] {
    auto boundary = surface;
    for (auto &point : surface) {
      signal_strength_[point]++;
      boundary.insert(neighbours(point));
    }
    surface = boundary;
  });
}

std::vector<double> forager::signals_at(location p) {
  return ranges::view::concat(
      ranges::view::repeat_n(1.0, signal_strength_[p]),
      ranges::view::repeat_n(0.0, sensor_range_ - signal_strength_[p]));
}

void forager::interact(life::signal output, location &p, direction &d,
                       double &score) {

  if (output.size() != 2) {
    std::cout
        << "Error: environment-forager must recieve an output of size 2\n";
    exit(1);
  }

  // outputs are interpreted as 0s and 1s only
  auto out = util::Bit(output[0]) * 2 + util::Bit(output[1]);

  // interact with the environment
  switch (out) {
  case 0: // move
    p = move_in_dir(p, d);
    break;
  case 1: // turn right
    d = turn(d, 3);
    break;
  case 2: // turn left
    d = turn(d, 1);
    break;
  case 3: // eat
    auto res = resources_.find(p);
    if (res != std::end(resources_)) {
      resources_.erase(res);
      if (replace_) {
        replace_resource_();
      }
      refresh_signals();
      score++;
    }
    break;
  }
}

double forager::eval(life::entity &org) {

  auto score = 0.0;
  auto p = location{std::rand() % grid_size_, std::rand() % grid_size_};
  auto d = direction{std::rand() % 4};

  resources_.clear();
  initialize_resource_();
  refresh_signals();

  util::repeat(updates_, [&] {
    // feed input to org; inputs are 0s and 1s only
    org.input(signals_at(p));
    // run the org once
    org.tick();
    // read its outputs and interact with the environment
    interact(org.output(), p, d, score);
  });

  return score;
}

void forager::evaluate(std::vector<life::entity> &pop) {

  for (auto &org : pop)
    org.data["score"] = eval(org);
}

