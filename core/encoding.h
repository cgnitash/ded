

#pragma once

#include "configuration.h"
#include "utilities.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace life {

class encoding {
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
  encoding() { configure(publish_configuration()); }

  configuration publish_configuration()
  {
    configuration con;
    con["parameters"]["alphabet"]          = alphabet_;
    con["parameters"]["copy-prob"]         = copy_prob_;
    con["parameters"]["copy-size"]         = copy_size_;
    con["parameters"]["del-prob"]          = del_prob_;
    con["parameters"]["del-size"]          = del_size_;
    con["parameters"]["point-insert-prob"] = point_insert_prob_;
    con["parameters"]["point-mutate-prob"] = point_mutate_prob_;
    con["parameters"]["point-delete-prob"] = point_delete_prob_;
    con["parameters"]["min-length"]        = min_length_;
    con["parameters"]["max-length"]        = max_length_;
    return con;
  }

  void configure(configuration con)
  {
    auto real = publish_configuration();
    validate_subset(con, real);
    merge_into(con, real);

    alphabet_          = con["parameters"]["alphabet"];
    copy_prob_         = con["parameters"]["copy-prob"];
    copy_size_         = con["parameters"]["copy-size"];
    del_prob_          = con["parameters"]["del-prob"];
    del_size_          = con["parameters"]["del-size"];
    point_insert_prob_ = con["parameters"]["point-insert-prob"];
    point_mutate_prob_ = con["parameters"]["point-mutate-prob"];
    point_delete_prob_ = con["parameters"]["point-delete-prob"];
    min_length_        = con["parameters"]["min-length"];
    max_length_        = con["parameters"]["max-length"];
  }

  auto begin() { return std::begin(enc_); }

  auto clear() { enc_.clear(); }

  auto push_back(long n) { enc_.push_back(n); }

  auto end() { return std::end(enc_); }

  auto &operator[](size_t i) { return enc_[i]; }

  size_t size() { return enc_.size(); }

  void generate(long size = 100)
  {
    enc_.clear();
    ranges::generate_n(ranges::back_inserter(enc_), size, [this] {
      return std::rand() % alphabet_;
    });
  }

  void copy_chunk()
  {
    auto copy_prob = std::rand() % 100;
    if (copy_prob < copy_prob_ * 100)
    {
      auto copy_from_pos = std::rand() % (enc_.size() - copy_size_);
      auto copy_to_pos   = std::rand() % enc_.size();
      auto copy_chunk =
          enc_ |
          ranges::view::slice(copy_from_pos, copy_from_pos + copy_size_) |
          ranges::copy;
      enc_.insert(std::begin(enc_) + copy_to_pos,
                  ranges::begin(copy_chunk),
                  ranges::end(copy_chunk));
    }
  }

  void del_chunk()
  {
    auto del_prob = std::rand() % 100;
    auto del_pos  = std::rand() % (enc_.size() - del_size_);
    if (del_prob < del_prob_ * 100)
      enc_.erase(std::begin(enc_) + del_pos,
                 std::begin(enc_) + del_pos + del_size_);
  }

  void point_mutate()
  {
    auto point_mut = enc_.size() * point_mutate_prob_;
    for (auto i = 0; i < point_mut; i++)
      enc_[std::rand() % enc_.size()] = std::rand() % alphabet_;
  }

  void point_insert()
  {
    auto point_ins = enc_.size() * point_insert_prob_;
    for (auto i = 0; i < point_ins; i++)
      enc_.insert(std::begin(enc_) + std::rand() % enc_.size(),
                  std::rand() % alphabet_);
  }

  void point_delete()
  {
    auto point_del = enc_.size() * point_delete_prob_;
    for (auto i = 0; i < point_del; i++)
      enc_.erase(std::begin(enc_) + std::rand() % enc_.size());
  }

  void all_deletions()
  {
    if (enc_.size() < min_length_) return;
    point_delete();
    del_chunk();
  }

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

  friend std::ostream &operator<<(std::ostream &o, const encoding &e)
  {
    for (auto &site : e.enc_) o << site << ":";
    return o;
  }
};
}   // namespace life
