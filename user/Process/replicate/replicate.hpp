#pragma once

#include "../../../components.hpp"

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

  ded::specs::ProcessSpec env_{ "null_process" };
  ded::concepts::Process      env = ded::makeProcess(env_);

public:
  replicate() { configure(publishConfiguration()); }

  ded::specs::ProcessSpec publishConfiguration()
  {
    ded::specs::ProcessSpec es;

    es.bindParameter("num", num_);

    es.bindPostTag("x", "double");

    es.bindProcess("env", env_);
    es.bindProcessPostConstraints("env", { { "x", "double" } });
    return es;
  }

  void configure(ded::specs::ProcessSpec es)
  {
    es.configureParameter("num", num_);

    es.configureProcess("env", env_);
    env = ded::makeProcess(env_);

    es.configurePostTag("x", x_tag_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
