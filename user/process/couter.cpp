
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

ded::population
    couter::evaluate(ded::population pop)
{

  for (const auto &org : pop.getAsVector())
    std::cout << (message_ ? "\nhi " : "\n ") << org.getID() << " "
              << std::get<double>(org.data.get_value(score_tag_));

  return pop;
}
