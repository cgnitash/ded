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

  ded::specs::EnvironmentSpec env_{"null_environment"};

public:
  pass_through() { configure(publish_configuration()); }

  ded::specs::EnvironmentSpec publish_configuration()
  {
    ded::specs::EnvironmentSpec es;

	es.bindEnvironment("env",env_);

    return es;
  }

  void configure(ded::specs::EnvironmentSpec es) { 
	  es.configureEnvironment("env",env_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population pop)
  {
    auto env = ded::make_Environment(env_);
    return env.evaluate(pop);
  }
};
