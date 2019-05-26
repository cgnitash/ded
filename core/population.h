
#pragma once

#include "encoding.h"
#include "entity.h"
#include "enhanced_type_traits.h"
#include "signal.h"
#include "specs/population_spec.h"

#include "configuration.h"
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace life {

// polymorphic wrapper for types that walk, talk, and quack like organisms
class population {
public:
  template <typename UserPopulation>
  population(UserPopulation x)
      : self_(new population_object<UserPopulation>(std::move(x)))
  {
  }

  population(const population &x) : data(x.data), self_(x.self_->copy_()) {}
  population(population &&) noexcept = default;

  population &operator=(const population &x)
  {
    population tmp(x);
    *this = std::move(tmp);
    return *this;
  }
  population &operator=(population &&) noexcept = default;

  // public interface of populations - how populations can be used
  life::configuration data;

  size_t size() const { return self_->size_(); }

  void merge(std::vector<entity> v) { self_->merge_(v); }

  void prune_lineage(long i) { self_->prune_lineage_(i); }

  void snapshot(long i) { self_->snapshot_(i); }

  std::vector<entity> get_as_vector() { return self_->get_as_vector_(); }

  void flush_unpruned() { self_->flush_unpruned(); }

  population_spec publish_configuration()
  {
    return self_->publish_configuration_();
  }

  void configure(population_spec es)
  {
    //auto real = publish_configuration();
    //validate_subset(con, real);
    //merge_into(con, real);
    self_->configure_(es);
  }

private:
  // interface/ABC for an population
  struct population_interface
  {
    virtual ~population_interface()             = default;
    virtual population_interface *copy_() const = 0;

    virtual population_spec publish_configuration_()  = 0;
    virtual void          configure_(population_spec) = 0;

    virtual size_t                    size_() const                     = 0;
    virtual std::vector<life::entity> get_as_vector_()                  = 0;
    virtual void                      merge_(std::vector<life::entity>) = 0;
    virtual void                      prune_lineage_(long)              = 0;
    virtual void                      snapshot_(long)                   = 0;
    virtual void                      flush_unpruned()                  = 0;
  };

  template <typename UserPopulation>
  struct population_object final : population_interface
  {

    // provided methods
    population_object(UserPopulation x) : data_(std::move(x)) {}

    population_interface *copy_() const override
    {
      return new population_object(*this);
    }

    // mandatory methods
    //

    void merge_(std::vector<entity> v) override { data_.merge(v); }

    std::vector<entity> get_as_vector_() override
    {
      return data_.get_as_vector();
    }

    size_t size_() const override { return data_.size(); }

    void flush_unpruned() override { data_.flush_unpruned(); }

    void prune_lineage_(long i) override { data_.prune_lineage(i); }

    void snapshot_(long i) override { data_.snapshot(i); }

    population_spec publish_configuration_() override
    {
      return data_.publish_configuration();
    }

    void configure_(population_spec c) override { data_.configure(c); }

    // optional methods

    // data
    UserPopulation data_;
  };

  std::unique_ptr<population_interface> self_;
};

}   // namespace life
