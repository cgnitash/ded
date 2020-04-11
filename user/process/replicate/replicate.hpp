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

class replicate
{

public:
  void
      configuration(ded::specs::ProcessSpec &spec)
  {
    spec.parameter("num", num_);

    spec.postTag("fx", "double");

    spec.nestedProcess("env", env_, {}, { { "x", "double" } });

    env = ded::makeProcess(env_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);

private:
  long num_ = 1;

  ded::specs::ProcessSpec env_{ "null_process" };
  ded::concepts::Process  env = ded::makeProcess(env_);
};
