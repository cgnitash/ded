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

class score_cout_printer {

  bool message_ = true;

  std::string used_score_tag_ = "score,double";

  life::configuration env_{ "null_environment", {} };

public:
  score_cout_printer() { configure(publish_configuration()); }

  life::configuration publish_configuration()
  {
    life::configuration c;
    c["parameters"]["message"] = message_;

    //  o:in:P must handle these tags
    c["pre-tags"] = nullptr;

    // o:in:P' has no tags
    c["post-tags"] = nullptr;

    c["input-tags"]  = nullptr;
    c["output-tags"] = nullptr;

    //  o:in:env(P) must provide these tags
    c["parameters"]["env"] = {
      env_[0], {}, {}, { used_score_tag_ }
    };   // as well as propogate population requirements

    return c;
  }

  void configure(life::configuration con)
  {
    message_ = con["parameters"]["message"];
    env_     = con["parameters"]["env"];
  }

  // guarantess org.data["x"] exists and is double
  life::population evaluate(life::population);
  // guarantess org.data["fx"] exists and is double
};
