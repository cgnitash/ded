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

class twoDio {

  std::string org_input_tag_  = "in,A<double,2>";
  std::string org_output_tag_ = "out,A<double,1>";

public:
  twoDio() { configure(publish_configuration()); }

  life::configuration publish_configuration()
  {
    life::configuration c;

    c["parameters"] = nullptr;

    c["pre-tags"] = nullptr;

    c["post-tags"] = nullptr;

    c["input-tags"]["in"]   = org_input_tag_;

    c["output-tags"]["out"] = org_output_tag_;

    return c;
  }

  void configure(life::configuration con) {
    org_input_tag_  = con["input-tags"]["in"];
    org_output_tag_ = con["output-tags"]["out"];
  }

  life::population evaluate(life::population);
};
