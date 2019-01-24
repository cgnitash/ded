
#include "bit_brain.h"

#include <algorithm>
#include <vector>

// this brain ignores any inputs
void bit_brain::input(std::string, life::signal) {}

// no internal state change on a tick
void bit_brain::tick() {}

void bit_brain::mutate()
{
  // only point mutations since the size of the encoding can't change
  genome_.point_mutate();
}

life::signal bit_brain::output(std::string n)
{
  if (n == out_sense_)
  {
    return std::vector<double>{ genome_ | ranges::view::transform(
                                              [](auto v) { return v % 2; }) };
  } else
  {
    std::cout << "Impl-Error: entity-markovbrain cannot handle this "
                 "name-signal pair in output \n";
    exit(1);
  }
}
