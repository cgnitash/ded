
#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "../configuration.h"
#include "../specs/population_spec.h"
#include "../utilities/tmp.h"
#include "encoding.h"
#include "entity.h"
#include "signal.h"

namespace ded
{
namespace concepts
{

// polymorphic wrapper for types that walk, talk, and quack like organisms
class Population
{
public:
  template <typename UserPopulation>
  Population(UserPopulation x)
      : self_(new PopulationObject<UserPopulation>(std::move(x)))
  {
  }

  Population(const Population &x) : data(x.data), self_(x.self_->copy_())
  {
  }
  Population(Population &&) noexcept = default;

  Population &
      operator=(const Population &x)
  {
    Population tmp(x);
    *this = std::move(tmp);
    return *this;
  }
  Population &operator=(Population &&) noexcept = default;

  // public interface of populations - how populations can be used
  // life::configuration data;
  std::map<std::string, specs::ConfigurationPrimitive> data;

  size_t
      size() const
  {
    return self_->size_();
  }

  void
      merge(std::vector<Entity> v)
  {
    self_->merge_(v);
  }

  void
      prune_lineage(long i)
  {
    self_->prune_lineage_(i);
  }

  void
      snapshot(long i)
  {
    self_->snapshot_(i);
  }

  std::vector<Entity>
      get_as_vector() const
  {
    return self_->get_as_vector_();
  }

  void
      flush_unpruned()
  {
    self_->flush_unpruned();
  }

  specs::PopulationSpec
      publish_configuration()
  {
    return self_->publish_configuration_();
  }

  void
      configure(specs::PopulationSpec es)
  {
    self_->configure_(es);
  }

private:
  // interface/ABC for an Population
  struct PopulationInterface
  {
    virtual ~PopulationInterface()             = default;
    virtual PopulationInterface *copy_() const = 0;

    virtual specs::PopulationSpec publish_configuration_()          = 0;
    virtual void                  configure_(specs::PopulationSpec) = 0;

    virtual size_t              size_() const               = 0;
    virtual std::vector<Entity> get_as_vector_() const      = 0;
    virtual void                merge_(std::vector<Entity>) = 0;
    virtual void                prune_lineage_(long)        = 0;
    virtual void                snapshot_(long)             = 0;
    virtual void                flush_unpruned()            = 0;
  };

  template <typename UserPopulation>
  struct PopulationObject final : PopulationInterface
  {

    // provided methods
    PopulationObject(UserPopulation x) : data_(std::move(x))
    {
    }

    PopulationInterface *
        copy_() const override
    {
      return new PopulationObject(*this);
    }

    // mandatory methods
    //

    void
        merge_(std::vector<Entity> v) override
    {
      data_.merge(v);
    }

    std::vector<Entity>
        get_as_vector_() const override
    {
      return data_.get_as_vector();
    }

    size_t
        size_() const override
    {
      return data_.size();
    }

    void
        flush_unpruned() override
    {
      data_.flush_unpruned();
    }

    void
        prune_lineage_(long i) override
    {
      data_.prune_lineage(i);
    }

    void
        snapshot_(long i) override
    {
      data_.snapshot(i);
    }

    specs::PopulationSpec
        publish_configuration_() override
    {
      auto ps  = data_.publish_configuration();
      ps.name_ = auto_class_name_as_string<UserPopulation>();
      return ps;
    }

    void
        configure_(specs::PopulationSpec c) override
    {
      data_.configure(c);
    }

    // optional methods

    // data
    UserPopulation data_;
  };

  std::unique_ptr<PopulationInterface> self_;
};

}   // namespace concepts
}   // namespace ded
