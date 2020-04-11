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

  ded::specs::ProcessSpec env_{"null_process"};

public:
  pass_through() { configure(publishConfiguration()); }

  ded::specs::ProcessSpec publishConfiguration()
  {
    ded::specs::ProcessSpec es;

	es.bindProcess("env",env_);

    return es;
  }

  void configure(ded::specs::ProcessSpec es) { 
	  es.configureProcess("env",env_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population pop)
  {
    auto env = ded::makeProcess(env_);
    return env.evaluate(pop);
  }
};
