
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

  std::regex comment_{R"(^\s*#.*)"};
  std::regex vary_command_{R"(^\s*vary\s+(.+)$)"};
  std::regex vary_with_{R"(^\s*with\s+(.+)$)"};

  auto check_lines() {
    return ranges::view::transform([this](auto line) {
      if (!(std::regex_match(line, vary_command_) ||
            std::regex_match(line, comment_) ||
            std::regex_match(line, vary_with_) || line.empty())) {
        std::cout << "error: unable to parse qst command " << line << "\n";
        std::exit(1);
      }
      return line;
    });
  }
  auto clean_lines() {
    return ranges::view::filter([this](auto line) {
      if (!(std::regex_match(line, vary_command_) ||
            std::regex_match(line, comment_) ||
            std::regex_match(line, vary_with_) || line.empty())) {
        std::cout << "error: unable to parse qst command " << line << "\n";
        std::exit(1);
      }
      return !(std::regex_match(line, comment_) || line.empty());
    });
  }

  auto by_vary() {
    return ranges::view::group_by([this](auto a, auto b) {
      return std::regex_match(a, vary_command_) ==
             std::regex_match(b, vary_command_);
    }) | ranges::view::chunk(2);
  }

  auto hash_experiment(std::string exp) {
    std::hash<std::string> h;
    return h(exp);
  }

  /*
  auto check_overrides() {
    return ranges::view::transform([this](auto over) {
      life::configuration true_exp = ranges::front(over);
      if (!ranges::all_of(
              *ranges::next(ranges::begin(over), 1),
              [this](auto with) { return can_override(true_exp, with); })) {
        std::cout << "error: some mismatch";
        exit(1);
      }
      return over;
    });
  }
  */

  auto expand_true_experiment() {
    return ranges::view::transform([this](auto vary) {
      std::smatch m;
      std::regex_match(ranges::front(ranges::front(vary)), m, vary_command_);
      auto p = life::global_path + m[1].str();
      return ranges::view::concat(
          ranges::view::single(true_user_experiment(p).dump()),
          *ranges::next(ranges::begin(vary), 1));
      //
      // vary ); 
    }) ;
  }
  

  life::configuration true_user_experiment(std::string);

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

