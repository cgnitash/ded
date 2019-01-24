
#include "toggle_bit_brain.h"

#include <algorithm>
#include <vector>

// this brain ignores any inputs
void toggle_bit_brain::input(std::string, life::signal) {}

// no internal state change on a tick
void
    toggle_bit_brain::tick()
{
}

void
    toggle_bit_brain::mutate()
{
  // only point mutations since the size of the encoding can't change
  genome_.point_mutate();
}

life::signal
    toggle_bit_brain::output(std::string n)
{
  if (n == out_as_true_)
  {
    return std::vector<double>{ genome_ | ranges::view::transform(
                                              [](auto v) { return v % 2; }) };
  } else if (n == out_as_false_)
  {
    return std::vector<double>{ genome_ | ranges::view::transform([](auto v) {
                                  return 1 - (v % 2);
                                }) };
  }
  std::exit(1);
}
