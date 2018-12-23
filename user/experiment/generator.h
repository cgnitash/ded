
#pragma once

#include "../../components.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

class generator {
  std::string file_;
  std::vector<life::configuration> parse();
  std::vector<life::configuration> all_exps_;

  std::regex vary_command_{R"(^\s*vary\s+(.+)$)"};

  auto clean_lines() {
    return ranges::view::filter([](auto line) {
      auto f = line.find_first_not_of(' ');
      return f != std::string::npos && line[f] != '#';
    });
  }

  auto by_vary() {
    return ranges::view::group_by([this](auto a, auto b) {
      return std::regex_match(a, vary_command_) ==
             std::regex_match(b, vary_command_);
    }) | ranges::view::chunk(2);
  }

  /*
  auto expand_experiment() {
	  return ranges::view::transform([this](auto vary) {
			  std::smatch m;
			  std::regex_match(vary.first,m,vary_command_);
			  return {true_user_experiment(m[1].str()),vary.second};
			  });
  }
  */

public:
  generator() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() {
   life::configuration con;
   con["file"] = file_;
   return con;
  }

  void configure(life::configuration con) {
    file_= con["file"];
  }

  void run();
};

