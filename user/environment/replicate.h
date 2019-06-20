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

  long num_ = 1;

  std::string x_tag_ = "x,double";

  life::environment_spec env_{ "null_environment" };
  life::environment      env = life::make_environment(env_);

public:
  replicate() { configure(publish_configuration()); }

  life::environment_spec publish_configuration()
  {
    life::environment_spec es{ "replicate" };

    es.bind_parameter("num", num_);

    es.bind_post("x", x_tag_);

    es.bind_environment("env", env_);
    es.bind_environment_post_constraints("env", { x_tag_ });
    return es;
  }

  void configure(life::environment_spec es)
  {
    es.configure_parameter("num", num_);

    es.configure_environment("env", env_);
    env = life::make_environment(env_);

    es.configure_post("x", x_tag_);
  }

  life::population evaluate(life::population);
};
