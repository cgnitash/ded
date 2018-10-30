
#include "bit_brain.h"

#include <algorithm>
#include <vector>

// this brain ignores any inputs
void bit_brain::input(life::signal) {}

// no internal state change on a tick
void bit_brain::tick() {}

void bit_brain::mutate() {
  // only point mutations since the size of the encoding can't change
  life::point_mutate(genome_);
}

life::signal bit_brain::output() {
  life::signal v;
  // encoding is simply the modulo 2 values
  std::transform(std::begin(genome_), std::end(genome_), std::back_inserter(v),
                 [](auto const value) { return value % 2; });
  return v;
}


