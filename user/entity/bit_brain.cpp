
#include"bit_brain.h"

#include <vector>
#include <algorithm>

  void bit_brain::mutate() {
    auto i = rand() % size_;
    encoding_[i] = !encoding_[i];
  }
  long bit_brain::update() const {
    return std::count(std::cbegin(encoding_), std::cend(encoding_), true);
  }


