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

    es.bindParameter("num", num_);

    es.bindPostTag("x", "double");

    es.bindEnvironment("env", env_);
    es.bindEnvironmentPostConstraints("env", { { "x", "double" } });
    return es;
  }

  void configure(ded::specs::EnvironmentSpec es)
  {
    es.configureParameter("num", num_);

    es.configureEnvironment("env", env_);
    env = ded::make_Environment(env_);

    es.configurePostTag("x", x_tag_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
