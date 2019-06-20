
#include "cycle.h"

#include <algorithm>
#include <experimental/filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

life::population
    cycle::evaluate(life::population pop)
{

  util::repeat(count_, [&] { pop = world.evaluate(pop); });

  return pop;
}
