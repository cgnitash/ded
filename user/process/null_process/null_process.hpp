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

class null_process
{

public:
  void
      configuration(ded::specs::ProcessSpec &)
  {
  }

  ded::concepts::Population
      evaluate(ded::concepts::Population pop)
  {
    return pop;
  }
};
