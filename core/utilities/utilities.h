
#pragma once

#include <range/v3/all.hpp>
#include <cmath>
#include <experimental/filesystem>
#include <fstream>

namespace ded {
namespace utilities {
double
    Bit(double d);

template <typename F>
void
    repeat(size_t n, F f)
{
  while (n--) f();
}

std::ofstream
    open_or_append(std::string file_name, std::string header);

}   // namespace util
}   // namespace util
