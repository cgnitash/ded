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

class pass_through {

  life::configuration env_{ "null_environment", {} };

public:
  pass_through() { configure(publish_configuration()); }

  life::configuration publish_configuration()
  {
    life::configuration c;
    c["pre-tags"] = nullptr;

    c["post-tags"]  = nullptr;

    c["input-tags"]  = nullptr;

    c["output-tags"] = nullptr;

    c["parameters"]["env"] = {
      env_[0], {}, {}, {}
    };   // and propogate population requirements

    return c;
  }

  void configure(life::configuration con) { env_ = con["parameters"]["env"]; }

  life::population evaluate(life::population);
};
