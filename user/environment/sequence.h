
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

class sequence {

  bool to_cout_ = false;

  std::string score_tag_ = "score,double";

  life::configuration one_{ "null_environment", {} };
  life::configuration two_{ "null_environment", {} };

public:
  sequence() { configure(publish_configuration()); }

  life::configuration publish_configuration()
  {
    life::configuration ec;

    ec["parameters"]["to-cout"] = to_cout_;

    ec["input-tags"]  = nullptr;
    ec["output-tags"] = nullptr;

    ec["pre-tags"]  = nullptr;
    ec["post-tags"] = nullptr;

    // o:in:selector(P) must handle these tags
    ec["parameters"]["one"] = { one_[0], {}, { score_tag_ }, {} };

    // o:in:fitness(P) must provide these tags
    ec["parameters"]["two"] = { two_[0], {}, {}, { score_tag_ } };

    return ec;
  }

  void configure(life::configuration con)
  {
    to_cout_ = con["parameters"]["to-cout"];

    one_ = con["parameters"]["one"];
    two_ = con["parameters"]["two"];
  }

  life::population evaluate(life::population);
};
