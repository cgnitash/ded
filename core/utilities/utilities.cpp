

#include <cmath>
#include <experimental/filesystem>
#include <fstream>
#include <range/v3/all.hpp>

#include "utilities.hpp"

namespace ded
{
namespace utilities
{
double
    Bit(double d)
{
  return d > 0. ? 1. : 0.;
}

std::ofstream
    open_or_append(std::string file_name, std::string header)
{
  std::ofstream file;
  if (!std::experimental::filesystem::exists(file_name))
  {
    file.open(file_name);
    file << header;
  }
  else
    file.open(file_name, std::ios::app);
  return file;
}

int
    closeness(std::string w1, std::string w2)
{
  return rs::inner_product(w1, w2, 0, std::plus<int>(), [](auto c1, auto c2) {
    return c1 != c2 ? 1 : 0;
  });
}

bool
    match(std::string attempt, std::string word)
{
  // In : abc
  // Out : [" abc","a bc","ab c","abc "]
  auto all_spaces = [](std::string word) {
    return rv::repeat_n(word, word.size() + 1) |
           rv::transform([n = -1](auto str) mutable {
             n++;
             return str.substr(0, n) + " " + str.substr(n);
           });
  };

  auto tolerance            = 3;
  auto [min, max, distance] = [w1 = attempt, w2 = word] {
    int l1 = w1.length();
    int l2 = w2.length();

    if (l1 < l2)
      return std::make_tuple(w1, w2, l2 - l1);
    if (l1 > l2)
      return std::make_tuple(w2, w1, l1 - l2);
    return std::make_tuple(w1, w2, 0);
  }();

  if (distance == 0)
  {
    // at most deletion + insertion, or at most 2 changes
    for (auto space_one : all_spaces(min))
      for (auto space_two : all_spaces(max))
        if (closeness(space_one, space_two) < tolerance)
          return true;
  }

  if (distance == 1)
  {
    // at most symmetrically (deletion + change, or insertion + change)
    for (auto spaced : all_spaces(min))
      if (closeness(spaced, max) < tolerance)
        return true;
  }
  if (distance == 2)
  {
    // at most symmetrically ( 2 deletions , or 2 insertions)
    for (auto spaced : all_spaces(min) | rv::transform([all_spaces](auto r) {
                         return all_spaces(r);
                       }) | rv::join)
      if (closeness(spaced, max) < tolerance)
        return true;
  }

  // if (distance >= tolerance)
  return false;
}

}   // namespace utilities
}   // namespace ded
