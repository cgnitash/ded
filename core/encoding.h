

#pragma once

#include <cstdlib>
#include <vector>

namespace life {

using encoding = std::vector<long>;

inline void copy_chunk(encoding &v, long prob = 100, long len = 20) {
  auto copy_prob = std::rand() % prob;
  if (!copy_prob) {
    auto copy_from_pos = std::rand() % (v.size() - len);
    auto copy_to_pos = std::rand() % v.size();
    life::encoding copy(std::begin(v) + copy_from_pos,
                        std::begin(v) + copy_from_pos + len);
    v.insert(std::begin(v) + copy_to_pos, std::begin(copy), std::end(copy));
  }
}

inline void del_chunk(encoding &v, long prob = 200, long len = 20) {
  auto del_prob = std::rand() % prob;
  auto del_pos = std::rand() % (v.size() - len);
  if (!del_prob)
    v.erase(std::begin(v) + del_pos, std::begin(v) + del_pos + len);
}

inline void point_mutate(encoding &v, long num = 3) {
  auto point_mut = rand() % num;
  for (auto i = 0; i < point_mut; i++)
    v[std::rand() % v.size()] = std::rand();
}

inline void point_insert(encoding &v, long num = 3) {
  auto point_ins = std::rand() % num;
  for (auto i = 0; i < point_ins; i++)
    v.insert(std::begin(v) + std::rand() % v.size(), std::rand());
}

inline void point_delete(encoding &v, long prob = 2) {
  auto point_del = std::rand() % prob;
  for (auto i = 0; i < point_del; i++)
    v.erase(std::begin(v) + std::rand() % v.size());
}
} // namespace life
