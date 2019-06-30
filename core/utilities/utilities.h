
#pragma once

#include <cmath>
#include <experimental/filesystem>
#include <fstream>
#include <range/v3/all.hpp>

namespace ded
{
namespace utilities
{
double Bit(double d);

template <typename F>
void
    repeat(size_t n, F f)
{
  while (n--)
    f();
}

std::ofstream open_or_append(std::string file_name, std::string header);

int  closeness(std::string w1, std::string w2);
bool match(std::string attempt, std::string word);
}   // namespace utilities
}   // namespace ded
