
#include "fork.h"

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
    fork::evaluate(life::population pop)
{
  auto one_pop = one.evaluate(pop);
  auto one_vec = one_pop.get_as_vector();

  auto two_pop = two.evaluate(pop);
  auto two_vec = two_pop.get_as_vector();

  // one and two don't change population
  // only data changes
  auto vec = pop.get_as_vector();
  for (auto &org : vec)
  {
    org.data.merge(ranges::find(one_vec, org)->data);
    org.data.merge(ranges::find(two_vec, org)->data);
  }
  pop.merge(vec);

  return pop;
}
