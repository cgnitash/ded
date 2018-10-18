

#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace life {

const auto al = 128;
using encoding = std::vector<long>;

inline encoding generate(long len = 100) {
  encoding v(len);
  std::generate(std::begin(v), std::end(v), [] { return std::rand() % al; });
  return v;
}

inline void copy_chunk(encoding &v, long prob = 5, long len = 20) {
  auto copy_prob = std::rand() % prob;
  if (!copy_prob) {
    auto copy_from_pos = std::rand() % (v.size() - len);
    auto copy_to_pos = std::rand() % v.size();
    life::encoding copy(std::begin(v) + copy_from_pos,
                        std::begin(v) + copy_from_pos + len);
    v.insert(std::begin(v) + copy_to_pos, std::begin(copy), std::end(copy));
  }
}

inline void del_chunk(encoding &v, long prob = 5, long len = 20) {
  auto del_prob = std::rand() % prob;
  auto del_pos = std::rand() % (v.size() - len);
  if (!del_prob)
    v.erase(std::begin(v) + del_pos, std::begin(v) + del_pos + len);
}

inline void point_mutate(encoding &v, double prob = 0.001) {
  auto point_mut = v.size() * prob;
  for (auto i = 0; i < point_mut; i++)
    v[std::rand() % v.size()] = std::rand() % al;
}

inline void point_insert(encoding &v, double prob = 0.001) {
  auto point_ins = v.size() * prob;
  for (auto i = 0; i < point_ins; i++)
    v.insert(std::begin(v) + std::rand() % v.size(), std::rand() % al);
}

inline void point_delete(encoding &v, double prob = 0.001) {
  auto point_del = v.size() * prob;
  for (auto i = 0; i < point_del; i++)
    v.erase(std::begin(v) + std::rand() % v.size());
}

inline void all_deletions(encoding &v) {
  if (v.size() < 50)
    return;
  point_delete(v);
  del_chunk(v);
}

inline void all_insertions(encoding &v) {
  if (v.size() > 500)
    return;
  point_insert(v);
  copy_chunk(v);
}

inline void all_mutations(encoding &v) {
  all_insertions(v);
  all_deletions(v);
  point_mutate(v);
}
} // namespace life
