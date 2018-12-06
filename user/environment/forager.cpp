
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

  size_t i, j;
  do {
    i = std::rand() % grid_size_;
    j = std::rand() % grid_size_;
  } while (resources_[i][j]);
  resources_[i][j] = 1;
}

std::vector<double> forager::signals_at(location loc, direction facing) {

  std::vector out(sensor_range_, 1.);
  for (auto i : util::rv3::view::iota(0u, sensor_range_)) {
    if (!resources_[loc.x_][loc.y_])
      out[i] = 0.;
    else
      break;
    loc = move_in_dir(loc, facing);
  }
  return out;
}

void forager::initialize_resource_() {

  resources_ = std::vector(grid_size_, std::vector(grid_size_, 0));
  for (auto i : util::rv3::view::iota(0u, grid_size_))
    for (auto j : util::rv3::view::iota(0u, grid_size_))
      if ((std::rand() % 1000) / 1000.0 < density_)
        resources_[i][j] = 1;
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
    facing = turn(facing, 3);
    break;
  case 2: // turn left
    facing = turn(facing, 1);
    break;
  case 3: // eat
    auto &pos = resources_[position.x_][position.y_];
    if (pos) {
      pos = 0;
      if (replace_) {
        replace_resource_();
      }
      score++;
    }
    break;
  }
}

double forager::eval(life::entity &org) {

  auto score = 0.0;
  // random starting position
  auto position = location{std::rand() % grid_size_, std::rand() % grid_size_};
  // face in random direction
  auto facing = direction{std::rand() % 4};

  resources_.clear();
  initialize_resource_();

  util::repeat(updates_, [&] {
    // feed input to org; inputs are 0s and 1s only
    org.input(signals_at(position, facing));
    // run the org once
    org.tick();
    // read its outputs and interact with the environment
    interact(org.output(), position, facing, score);
  });

  return score;
}

life::population forager::evaluate(life::population pop) {
  pop.merge(pop.get_as_vector() |
            util::rv3::action::transform([this](auto &org) {
              org.data["score"] = eval(org);
              return org;
            }));
  return pop;
}

