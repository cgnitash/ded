

#include "encoding.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace ded
{
namespace concepts
{
void
    Encoding::generate(long size)
{
  enc_.clear();
  rs::generate_n(rs::back_inserter(enc_), size, [this] {
    return std::rand() % alphabet_;
  });
}

void
    Encoding::copy_chunk()
{
  auto copy_prob = std::rand() % 100;
  if (copy_prob < copy_prob_ * 100)
  {
    auto copy_from_pos = std::rand() % (enc_.size() - copy_size_);
    auto copy_to_pos   = std::rand() % enc_.size();
    auto copy_chunk =
        enc_ | rv::slice(copy_from_pos, copy_from_pos + copy_size_) |
        rs::copy;
    enc_.insert(std::begin(enc_) + copy_to_pos,
                rs::begin(copy_chunk),
                rs::end(copy_chunk));
  }
}

void
    Encoding::del_chunk()
{
  auto del_prob = std::rand() % 100;
  auto del_pos  = std::rand() % (enc_.size() - del_size_);
  if (del_prob < del_prob_ * 100)
    enc_.erase(std::begin(enc_) + del_pos,
               std::begin(enc_) + del_pos + del_size_);
}

void
    Encoding::point_mutate()
{
  auto point_mut = enc_.size() * point_mutate_prob_;
  for (auto i = 0; i < point_mut; i++)
    enc_[std::rand() % enc_.size()] = std::rand() % alphabet_;
}

void
    Encoding::point_insert()
{
  auto point_ins = enc_.size() * point_insert_prob_;
  for (auto i = 0; i < point_ins; i++)
    enc_.insert(std::begin(enc_) + std::rand() % enc_.size(),
                std::rand() % alphabet_);
}

void
    Encoding::point_delete()
{
  auto point_del = enc_.size() * point_delete_prob_;
  for (auto i = 0; i < point_del; i++)
    enc_.erase(std::begin(enc_) + std::rand() % enc_.size());
}

void
    Encoding::all_deletions()
{
  if (enc_.size() < min_length_)
    return;
  point_delete();
  del_chunk();
}

}   // namespace concepts
}   // namespace ded
