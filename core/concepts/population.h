
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

  Population(const Population &x) : self_(x.self_->copy_())
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
  // std::map<std::string, specs::ConfigurationPrimitive> data;

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

    template <typename T>
    using HasMerge = decltype(
        std::declval<T &>().merge(std::declval<std::vector<Entity>>()));
    static_assert(
        utilities::TMP::has_signature<UserPopulation, void, HasMerge>{},
        "UserPopulation does not satisfy 'merge' concept requirement");
    void
        merge_(std::vector<Entity> v) override
    {
      data_.merge(v);
    }

    template <typename T>
    using HasGetAsVector = decltype(std::declval<T &>().get_as_vector());
    static_assert(
        utilities::TMP::has_signature<UserPopulation,
                                      std::vector<Entity>,
                                      HasGetAsVector>{},
        "UserPopulation does not satisfy 'get_as_vector' concept requirement");
    std::vector<Entity>
        get_as_vector_() const override
    {
      return data_.get_as_vector();
    }

    template <typename T>
    using HasSize = decltype(std::declval<T &>().size());
    static_assert(
        utilities::TMP::has_signature<UserPopulation, size_t, HasSize>{},
        "UserPopulation does not satisfy 'size' concept requirement");
    size_t
        size_() const override
    {
      return data_.size();
    }

    template <typename T>
    using HasFlush = decltype(std::declval<T &>().flush_unpruned());
    static_assert(
        utilities::TMP::has_signature<UserPopulation, void, HasFlush>{},
        "UserPopulation does not satisfy 'flush_unpruned' concept requirement");
    void
        flush_unpruned() override
    {
      data_.flush_unpruned();
    }

    template <typename T>
    using HasPruneLineage =
        decltype(std::declval<T &>().prune_lineage(std::declval<long>()));
    static_assert(
        utilities::TMP::has_signature<UserPopulation, void, HasPruneLineage>{},
        "UserPopulation does not satisfy 'prune_lineage' concept requirement");
    void
        prune_lineage_(long i) override
    {
      data_.prune_lineage(i);
    }

    template <typename T>
    using HasSnapshot =
        decltype(std::declval<T &>().snapshot(std::declval<long>()));
    static_assert(
        utilities::TMP::has_signature<UserPopulation, void, HasSnapshot>{},
        "UserPopulation does not satisfy 'snapshot' concept requirement");
    void
        snapshot_(long i) override
    {
      data_.snapshot(i);
    }

    template <typename T>
    using HasConf = decltype(
        std::declval<T &>().configure(std::declval<specs::PopulationSpec>()));
    template <typename T>
    using HasPubConf = decltype(std::declval<T &>().publish_configuration());
    static_assert(
        utilities::TMP::has_signature<UserPopulation, void, HasConf>{} &&
            utilities::TMP::
                has_signature<UserPopulation, specs::PopulationSpec, HasPubConf>{},
        "UserPopulation does not satisfy 'configuration' concept requirement");
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
