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

class null_environment {

public:
  null_environment() { configure(publish_configuration()); }

  ded::specs::EnvironmentSpec publish_configuration()
  {
	return {};
  }

  void configure(ded::specs::EnvironmentSpec) {}

  ded::concepts::Population evaluate(ded::concepts::Population pop) { return pop; }
};
