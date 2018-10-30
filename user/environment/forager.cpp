
#include "forager.h"
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


void forager::replace_resource_() {
  while (true) {
    auto p = location{std::rand() % grid_size_, std::rand() % grid_size_};
    auto inserted = resources_.insert(p);
    if (inserted.second)
      break;
  }
}

void forager::initialize_resource_() {

  for (auto i{0u}; i < grid_size_; i++)
    for (auto j{0u}; j < grid_size_; j++)
      if ((std::rand() % 100) / 100.0 < density_)
        resources_.insert(location{i, j});

}

void forager::refresh_signals() {

  signal_strength_.clear();
  auto surface = resources_;

  for (auto i{0u}; i < sensor_range_; i++) {
    auto boundary = surface;
    for (auto &point : surface) {
      signal_strength_[point]++;
      boundary.insert(neighbours(point));
    }
    surface = boundary;
  }
}

std::vector<double> forager::signals_at(location p) {
  // inputs to be fed to org 
  auto v = std::vector(sensor_range_, 0.0);
  std::fill_n(std::begin(v), signal_strength_[p], 1.0);
  return v;
}

double forager::eval(life::entity &org) {

  auto score = 0.0;
  auto p = location{std::rand() % grid_size_, std::rand() % grid_size_};
  auto d = direction{std::rand() % 4};

  resources_.clear();
  initialize_resource_();
  refresh_signals();

  for (auto i{0u}; i < updates_; i++) {
    // feed input to org; inputs are 0s and 1s only
    org.input(signals_at(p));
    // run the org once
    org.tick();

    // read its outputs
    auto output = org.output();
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
  return score;
}

void forager::evaluate(std::vector<life::entity> &pop) {

  for (auto &org : pop)
    org.data["score"] = eval(org);
}

