
#pragma once

#include"range-v3/all.hpp"

#include<cmath>

namespace util {
inline double Bit(double d) { return d > 0. ? 1. : 0.; }

const auto PI = std::atan(1) * 4;

template <typename F> void repeat(size_t n, F f) {
  while (n--)
    f();
}

} // namespace util
