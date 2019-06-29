

#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "../configuration.h"
#include "../utilities/utilities.h"

namespace ded {
namespace concepts {

class Encoding {
  size_t alphabet_{ 128 };
  double copy_prob_{ 0.01 };
  size_t del_size_{ 20 };
  double del_prob_{ 0.01 };
  size_t copy_size_{ 20 };
  double point_insert_prob_{ 0.01 };
  double point_mutate_prob_{ 0.01 };
  double point_delete_prob_{ 0.01 };
  size_t min_length_{ 50 };
  size_t max_length_{ 2500 };

  std::vector<long> enc_;

public:

  auto begin() { return std::begin(enc_); }

  auto clear() { enc_.clear(); }

  auto push_back(long n) { enc_.push_back(n); }

  auto end() { return std::end(enc_); }

  auto &operator[](size_t i) { return enc_[i]; }

  size_t size() { return enc_.size(); }

  void generate(long = 100);

  void copy_chunk();

  void del_chunk();

  void point_mutate();

  void point_insert();

  void point_delete();

  void all_deletions();

  inline void all_insertions()
  {
    if (enc_.size() > max_length_) return;
    point_insert();
    copy_chunk();
  }

  inline void all_mutations()
  {
    all_insertions();
    all_deletions();
    point_mutate();
  }

  friend std::ostream &operator<<(std::ostream &o, const Encoding &e)
  {
    for (auto &site : e.enc_) o << site << ":";
    return o;
  }
};
}   // namespace ded
}   // namespace ded
