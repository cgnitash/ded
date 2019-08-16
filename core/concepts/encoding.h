

#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "../configuration.h"
#include "../utilities/utilities.h"

namespace ded
{
namespace concepts
{

class Encoding
{
  long alphabet_{ 128 };
  double copy_prob_{ 0.01 };
  long del_size_{ 20 };
  double del_prob_{ 0.01 };
  long copy_size_{ 20 };
  double point_insert_prob_{ 0.01 };
  double point_mutate_prob_{ 0.01 };
  double point_delete_prob_{ 0.01 };
  long min_length_{ 50 };
  long max_length_{ 2500 };

  std::vector<long> enc_;

public:
  Encoding() { configure(publishConfiguration()); }

  ded::specs::EncodingSpec publishConfiguration()
  {
    ded::specs::EncodingSpec es;

	es.bindParameter("alphabet",alphabet_);
	es.bindParameter("copy_prob",copy_prob_);
	es.bindParameter("del_size",del_size_);
	es.bindParameter("del_prob",del_prob_);
	es.bindParameter("copy_size",copy_size_);
	es.bindParameter("point_insert_prob",point_insert_prob_);
	es.bindParameter("point_mutate_prob",point_mutate_prob_);
	es.bindParameter("point_delete_prob",point_delete_prob_);
	es.bindParameter("min_length",min_length_);
	es.bindParameter("max_length",max_length_);

    return es;
  }
  
  void configure(ded::specs::EncodingSpec es)
  {
	es.configureParameter("alphabet",alphabet_);
	es.configureParameter("copy_prob",copy_prob_);
	es.configureParameter("del_size",del_size_);
	es.configureParameter("del_prob",del_prob_);
	es.configureParameter("copy_size",copy_size_);
	es.configureParameter("point_insert_prob",point_insert_prob_);
	es.configureParameter("point_mutate_prob",point_mutate_prob_);
	es.configureParameter("point_delete_prob",point_delete_prob_);
	es.configureParameter("min_length",min_length_);
	es.configureParameter("max_length",max_length_);
  }

  auto
      begin() const
  {
    return std::begin(enc_);
  }

  auto
      clear()
  {
    enc_.clear();
  }

  auto
      push_back(long n)
  {
    enc_.push_back(n);
  }

  auto
      end() const
  {
    return std::end(enc_);
  }

  auto &operator[](size_t i) const
  {
    return enc_[i];
  }

  size_t
      size() const
  {
    return enc_.size();
  }

  void seed_codons(std::vector<long> const &codon, size_t n) 
  {
    ded::utilities::repeat(n, [&] {
      auto pos = std::rand() % (enc_.size() - 1);
      std::copy(std::begin(codon), std::end(codon), std::begin(enc_) + pos);
    });
  }
  void generate(long = 100);

  void copy_chunk();

  void del_chunk();

  void point_mutate();

  void point_insert();

  void point_delete();

  void all_deletions();

  inline void
      all_insertions()
  {
    if (enc_.size() > static_cast<size_t>(max_length_))
      return;
    point_insert();
    copy_chunk();
  }

  inline void
      all_mutations()
  {
    all_insertions();
    all_deletions();
    point_mutate();
  }

  friend std::ostream &
      operator<<(std::ostream &o, const Encoding &e)
  {
    for (auto const &site : e.enc_)
      o << site << ":";
    return o;
  }
};
}   // namespace concepts
}   // namespace ded
