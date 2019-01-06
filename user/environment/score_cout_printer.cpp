
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

life::population score_cout_printer::evaluate(life::population pop) {

  // make and configure nested environment
  auto env = life::make_environment(env_name_);
  env.configure(env_config_);

  for (const auto &org : env.evaluate(pop).get_as_vector())
    std::cout << (message_ ? "\nhi " : "\n ") << org.get_id() << " "
              << std::get<double>(org.data.get_value(used_score_tag_));

  return pop;
}

