
#include "forager.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

void forager::replace_resource_() {
  while (true){
  // yucky randomness :(
    auto p = location{std::rand() % grid_size_, std::rand() % grid_size_};
    auto  inserted = resources_.insert(p);
    if (inserted.second)
      break;
  }
}

void forager::refresh_signals() {

  signal_strength_.clear();
  std::set<location> signalable;
  for (auto &p : resources_)
    signalable.insert(p);

  for (auto i = 0; i < 4; i++) {
    std::set<location> temp;
    for (auto &p : signalable) {
      signal_strength_[p]++;
      temp.insert(neighbours(p));
    }
    signalable = temp;
  }
}

double forager::eval(life::entity org) {

  auto score = 0.0;
  resources_.clear();
  auto p = location{std::rand() % grid_size_, std::rand() % grid_size_};
  auto d = direction{std::rand() % 4};

  for (auto i = 0; i < density_ * grid_size_ * grid_size_; i++)
	 replace_resource_(); 
  
  for (auto i = 0; i < updates_; i++) {
    // feed input to org
    switch (signal_strength_[p]) {
    case 0:
      org.input({0, 0, 0, 0});
      break;
    case 1:
      org.input({1, 0, 0, 0});
      break;
    case 2:
      org.input({1, 1, 0, 0});
      break;
    case 3:
      org.input({1, 1, 1, 0});
      break;
    case 4:
      org.input({1, 1, 1, 1});
      break;
    }

    // run the org once
    org.tick();

    // read its outputs
    auto output = org.output();
    auto out = static_cast<long>(output[0]) * 2 + static_cast<long>(output[1]);
    // interact with the environment
    switch (out) {
    case 0: // move
      p = move_in_dir(p, d);
      break;
    case 1: // turn right
      d = turn(d, 3);
      break;
    case 2: // turn right
      d = turn(d, 1);
      break;
    case 3: // eat
      auto res = resources_.find(p);
      if (res != std::end(resources_)) {
        resources_.erase(res);
        score++;
        replace_resource_();
        refresh_signals();
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

