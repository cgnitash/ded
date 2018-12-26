
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
  std::vector<life::configuration> all_exps_;

  std::regex comment_{R"(^\s*#.*)"};
  std::regex spaces_{R"(\s+)"};
  std::regex vary_command_{R"(^\s*vary\s+(.+)$)"};
  std::regex vary_with_{
      R"(^\s*with\s*(\s[^\[]+\S)\s*\[\s*([^\]]+\S)\s*\]\s*$)"};


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


  auto generate_overrides(life::configuration con) {
    return ranges::view::transform([this, con](auto patch_options) {
      std::smatch m;
      std::regex_match(patch_options, m, vary_with_);
	  
      auto patch_path = std::regex_replace(m[1].str(), spaces_, "/1/");
      check_parameter_override_error(con, patch_path);

      auto jar = life::configuration::parse(
          std::string{"["} + std::regex_replace(m[2].str(), spaces_, ",") +
          std::string{"]"});
      auto p = con.at(life::configuration::json_pointer{patch_path});
      check_jar_types(jar, patch_path, p.type_name());

      std::vector<std::pair<std::string, life::configuration>> v =
          ranges::view::zip_with(
              [](auto w, auto l) {
                return std::make_pair(w, life::configuration{l});
              },
              ranges::view::repeat_n(patch_path, jar.size()), jar);
      return v;
    });
  }

  auto by_vary() {
    return ranges::view::group_by([this](auto line1, auto line2) {
             return std::regex_match(line1, vary_command_) ==
                    std::regex_match(line2, vary_command_);
           }) |
           ranges::view::chunk(2) | ranges::view::transform([this](auto p) {
             std::smatch m;
             std::regex_match(ranges::front(ranges::front(p)), m,
                              vary_command_);
             auto con = true_user_experiment(life::global_path + m[1].str());
             std::vector<
                 std::vector<std::pair<std::string, life::configuration>>>
                 overs = *ranges::next(ranges::begin(p), 1) |
                         generate_overrides(con);
             return std::make_pair(con, overs);
           });
  }

  auto hash_experiment(std::string exp) {
    std::hash<std::string> h;
    return h(exp);
  }

  auto
  apply_unchecked_override(life::configuration con,
                           std::pair<std::string, life::configuration> pv) {

    con.at(life::configuration::json_pointer{pv.first}) = pv.second[0];
    return con;
  }

  auto expand_true_experiment() {
    return ranges::view::transform([this](auto exp) {
      std::vector<life::configuration> ret_exp;
      ret_exp.push_back(exp.first);
      return ranges::accumulate(
          exp.second , ret_exp,
          [this](auto b, auto s) {
            std::vector<life::configuration> w;
            std::vector<std::pair<std::string, life::configuration>> m = s;
            ranges::transform(ranges::view::cartesian_product(b, m),
                              ranges::back_inserter(w), [this](const auto &t) {
                                return apply_unchecked_override(std::get<0>(t),
                                                                std::get<1>(t));
                              });
            return w;
          });
    });
  }

  void check_parameter_override_error(life::configuration, std::string);
  void check_jar_types(life::configuration, std::string, std::string);

  life::configuration true_user_experiment(std::string);

public:
  generator() { configure(publish_configuration()); }

  life::configuration publish_configuration() {
    life::configuration con;
    con["file"] = file_;
    return con;
  }

  void configure(life::configuration con) { file_ = con["file"]; }

  void run();
};

