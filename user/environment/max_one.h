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

class max_one {

  std::string ones_tag_ = "ones,double";
  std::string org_output_ones_tag_ = "ones,A<double>";

public:
  max_one() { configure(publish_configuration()); }

  life::configuration publish_configuration() {
    life::configuration c;
    c["parameters"] = nullptr;

    //  o:in:P must handle these tags
    c["pre-tags"] = nullptr;

    // o:in:P' must handle these tags
    c["post-tags"]["ones"] = ones_tag_;

    c["input-tags"] = nullptr;
    c["output-tags"]["ones"] = org_output_ones_tag_;

    return c;
  }

  void configure(life::configuration c) {
    ones_tag_ = c["post-tags"]["ones"];
    org_output_ones_tag_ = c["output-tags"]["ones"];
  }

  life::population evaluate(life::population);
};

