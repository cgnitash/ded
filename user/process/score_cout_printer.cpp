
#include "score_cout_printer.h"

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
    score_cout_printer::evaluate(ded::population pop)
{

  // make and configure nested process
  auto env = ded::makeprocess(env_);

  for (const auto &org : env.evaluate(pop).getAsVector())
    std::cout << (message_ ? "\nhi " : "\n ") << org.getID() << " "
              << std::get<double>(org.data.get_value(used_score_tag_));

  return pop;
}
