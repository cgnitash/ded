
#include "couter.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

life::population
    couter::evaluate(life::population pop)
{

  for (const auto &org : pop.get_as_vector())
    std::cout << (message_ ? "\nhi " : "\n ") << org.get_id() << " "
              << std::get<double>(org.data.get_value(score_tag_));

  return pop;
}
