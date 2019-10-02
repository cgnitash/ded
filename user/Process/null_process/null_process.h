#pragma once

#include "../../../components.h"

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

class null_process {

public:
  null_process() { configure(publishConfiguration()); }

  ded::specs::ProcessSpec publishConfiguration()
  {
	return {};
  }

  void configure(ded::specs::ProcessSpec) {}

  ded::concepts::Population evaluate(ded::concepts::Population pop) { return pop; }
};
