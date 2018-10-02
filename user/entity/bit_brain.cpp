
#include"bit_brain.h"

#include <vector>
#include <algorithm>

  void bit_brain::tick() { }

  void bit_brain::mutate() {
    auto i = rand() % size_;
    encoding_[i] = !encoding_[i];
  }

  life::signal bit_brain::output() {
    return {static_cast<double>(
        std::count(std::cbegin(encoding_), std::cend(encoding_), true))};
  }
void bit_brain::input(life::signal) {}
