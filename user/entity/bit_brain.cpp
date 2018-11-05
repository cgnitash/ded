
#include "bit_brain.h"
#include "../../core/utilities.h"

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
  return genome_ | util::rv3::view::transform([](auto v) { return v % 2; });
}

