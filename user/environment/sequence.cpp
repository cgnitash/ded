
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


  pop = first.evaluate(pop);

  pop = second.evaluate(pop);

  return pop;
}
