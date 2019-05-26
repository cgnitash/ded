
#include "sequence.h"

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
    sequence::evaluate(life::population pop)
{

  auto one = life::make_environment(one_);

  auto two = life::make_environment(two_);

  pop = one.evaluate(pop);

  /*
  if (to_cout_)
  {
    auto vec = pop;
    for (const auto &org : vec.get_as_vector())
      std::cout << "id: " << org.get_id() << " : "
                << std::get<double>(org.data.get_value(score_tag_))
                << std::endl;
  }
	*/

  pop = two.evaluate(pop);

  return pop;
}
