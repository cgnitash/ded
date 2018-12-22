
#pragma once

#include "csv/CSV.h"
#include "range-v3/all.hpp"

#include <cmath>
#include <experimental/filesystem>
#include <fstream>

namespace util {
inline double Bit(double d) { return d > 0. ? 1. : 0.; }

const auto PI = std::atan(1) * 4;

template <typename F> void repeat(size_t n, F f) {
  while (n--)
    f();
}

inline std::ofstream open_or_append(std::string file_name, std::string header) {
  std::ofstream file;
  if (!std::experimental::filesystem::exists(file_name)) {
    file.open(file_name);
    file << header;
  } else
    file.open(file_name, std::ios::app);
  return file;
}

// In : abc
// Out : [bc,ac,ab]
inline auto deletes(std::string word) {
  return ranges::view::repeat_n(word, word.size()) |
         ranges::view::transform([n = 0](auto str) mutable {
           n++;
           return str.substr(0, n - 1) + str.substr(n);
         });
}

// In : xabc
// Out : [xabc,axbc,abxc,abcx]
inline auto rotated_inserts(std::string word) {
  return ranges::view::repeat_n(word, word.size()) |
         ranges::view::transform([n = 0](auto str) mutable {
           n++;
           return str.substr(1, n - 1) + std::string{str[0]} + str.substr(n);
         });
}

// In : pqr
// Out : [apqr,bpqr,cpqr ... zpqr]
inline auto pad_words(std::string word) {
  return ranges::view::zip_with(
      [](auto l, auto w) { return l + w; }, ranges::view::closed_iota('a', 'z'),
      ranges::view::single(word) | ranges::view::cycle);
}

// In: hi
// Out : [ahi,hai,hia,bhi ... hiz]
inline auto inserts(std::string word) {
  return pad_words(word) | ranges::view::transform([](auto padded_word) {
           return rotated_inserts(padded_word);
         }) |
         ranges::view::join;
}

// In : xabc
// Out : [xbc,axc,abx]
inline auto rotated_changes(std::string word) {
  return ranges::view::repeat_n(word, word.size() - 1) |
         ranges::view::transform([n = 0](auto str) mutable {
           n++;
           return str.substr(1, n - 1) + std::string{str[0]} +
                  str.substr(n + 1);
         });
}

// In: hi
// Out : [ai,ha,bi ... hz]
inline auto changes(std::string word) {
  return pad_words(word) | ranges::view::transform([](auto padded_word) {
           return rotated_changes(padded_word);
         }) |
         ranges::view::join;
}

inline auto all_edits() {
  return ranges::view::transform([](auto word) {
           return ranges::view::concat(deletes(word), changes(word),
                                       inserts(word));
         }) |
         ranges::view::join;
}
} // namespace util
