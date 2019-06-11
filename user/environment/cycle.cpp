
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


  for (auto i : ranges::view::iota(0, count_))
  {

    pop = world.evaluate(pop);

    //if (snap_freq_ && !(i % snap_freq_)) pop.snapshot(i);

    // this should not be necessary to say
    pop.prune_lineage(i);
  }
  // this should not be necessary to say
  pop.flush_unpruned();

  return pop;
}
