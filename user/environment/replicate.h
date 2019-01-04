#pragma once

#include "../../components.h"

#include <algorithm>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <vector>

class replicate {

  size_t num_ = 1;

  std::string fx_tag_ = "fx";
  std::string used_x_tag_ = "x";

  life::configuration env_ {"null_environment",{}};

public:
  replicate() { configure(publish_configuration()); }

  life::configuration publish_configuration() {
    life::configuration c;
    c["parameters"]["num"] = num_;

    // o:in:P has no tags
    c["pre-tags"] = nullptr;

    //  o:in:P' must handle these tags
    c["post-tags"]["fx"] = fx_tag_;

    //  o:in:env(P) must provide these tags
    c["parameters"]["env"] = {
        env_[0],
        {},
        {},
        {used_x_tag_}}; // as well as propogate population requirements

    return c;
  }

  void configure(life::configuration con) {
    num_ = con["parameters"]["num"];
    env_= con["parameters"]["env"];

    fx_tag_ = con["post-tags"]["fx"];
  }

  life::population evaluate(life::population);
};

