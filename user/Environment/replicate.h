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

  std::string x_tag_ = "double";

  ded::specs::EnvironmentSpec env_{ "null_environment" };
  ded::concepts::Environment      env = ded::make_Environment(env_);

public:
  replicate() { configure(publish_configuration()); }

  ded::specs::EnvironmentSpec publish_configuration()
  {
    ded::specs::EnvironmentSpec es;

    es.bind_parameter("num", num_);

    es.bind_post("x", "double");

    es.bind_environment("env", env_);
    es.bind_environment_post_constraints("env", { "double" });
    return es;
  }

  void configure(ded::specs::EnvironmentSpec es)
  {
    es.configure_parameter("num", num_);

    es.configure_environment("env", env_);
    env = ded::make_Environment(env_);

    es.configure_post("x", x_tag_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
