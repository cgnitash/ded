
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

  for (size_t i : util::rv3::view::iota(0, grid_size_))
    for (size_t j : util::rv3::view::iota(0, grid_size_))
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
  return util::rv3::view::concat(
      util::rv3::view::repeat_n(1.0, signal_strength_[p]),
      util::rv3::view::repeat_n(0.0, sensor_range_ - signal_strength_[p]));
}

void forager::interact(life::signal output, location &position,
                       direction &facing, double &score) {

  if (output.size() != 2) {
    std::cout
        << "Error: environment-forager must recieve an output of size 2\n";
    exit(1);
  }

  // outputs are interpreted as 0s and 1s only
  auto out = util::Bit(output[0]) * 2 + util::Bit(output[1]);

  // interact with the environment
  switch (static_cast<int>(out)) {
  case 0: // move
    position = move_in_dir(position, facing);
    break;
  case 1: // turn right
    facing  = turn(facing, 3);
    break;
  case 2: // turn left
    facing = turn(facing, 1);
    break;
  case 3: // eat
    auto res = resources_.find(position);
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
  auto position = location{std::rand() % grid_size_, std::rand() % grid_size_};
  auto facing = direction{std::rand() % 4};

  resources_.clear();
  initialize_resource_();
  refresh_signals();

  util::repeat(updates_, [&] {
    // feed input to org; inputs are 0s and 1s only
    org.input(signals_at(position));
    // run the org once
    org.tick();
    // read its outputs and interact with the environment
    interact(org.output(), position, facing, score);
  });

  return score;
}

std::vector<life::entity>
forager::evaluate(const std::vector<life::entity> &pop) {
  auto new_pop = pop;
  for (auto &org : new_pop)
    org.data["score"] = eval(org);
  return new_pop;
}

