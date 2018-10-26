
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

void forager::remove_resource_() {
  while (true) {
    auto p = location{std::rand() % grid_size_, std::rand() % grid_size_};
    if (resources_.erase(p))
      break;
  }
}

void forager::replace_resource_() {
  while (true) {
    auto p = location{std::rand() % grid_size_, std::rand() % grid_size_};
    auto inserted = resources_.insert(p);
    if (inserted.second)
      break;
  }
}

void forager::initialize_resource_() {

  if (density_ < 0.5) {
    for (auto i = 0; i < density_ * grid_size_ * grid_size_; i++)
      replace_resource_();
  } else {
    for (auto i = 0; i < grid_size_; i++)
      for (auto j = 0; j < grid_size_; j++)
        resources_.insert(location{i, j});
    for (auto i = 0; i < density_ * grid_size_ * grid_size_; i++)
      remove_resource_();
  }
}

void forager::refresh_signals() {

  signal_strength_.clear();
  auto surface = resources_;

  for (auto i = 0; i < 4; i++) {
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
  auto v = std::vector<double>(4, 0);
  std::fill_n(std::begin(v), signal_strength_[p], 1);
  return v;
}

double forager::eval(life::entity org) {

  auto score = 0.0;
  auto p = location{std::rand() % grid_size_, std::rand() % grid_size_};
  auto d = direction{std::rand() % 4};

  resources_.clear();
  initialize_resource_();
  refresh_signals();

  for (auto i = 0; i < updates_; i++) {
    // feed input to org
    org.input(signals_at(p));
    // run the org once
    org.tick();

    // read its outputs
    auto output = org.output();
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

life::eval_results forager::evaluate(const std::vector<life::entity> &pop) {

  life::eval_results scores;
  std::transform(std::begin(pop), std::end(pop), std::back_inserter(scores),
                 [this](auto &org) {
                   std::map<std::string, std::string> m;
                   m["score"] = std::to_string(eval(org));
                   return std::make_pair(org, m);
                 });
  return scores;
}

