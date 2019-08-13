
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
#include "substrate.h"
#include "signal.h"

namespace ded
{
namespace concepts
{

// polymorphic wrapper for Population
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
      merge(std::vector<Substrate> v)
  {
    self_->merge_(v);
  }

  void
      pruneLineage(long i)
  {
    self_->pruneLineage_(i);
  }

  void
      snapShot(long i)
  {
    self_->snapShot_(i);
  }

  std::vector<Substrate>
      getAsVector() const
  {
    return self_->getAsVector_();
  }

  void
      flushUnpruned()
  {
    self_->flushUnpruned_();
  }

  specs::PopulationSpec
      publishConfiguration()
  {
    return self_->publishConfiguration_();
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
  public:
    // provided methods
    virtual ~PopulationInterface()             = default;
    virtual PopulationInterface *copy_() const = 0;

    // mandatory methods
    virtual specs::PopulationSpec publishConfiguration_()          = 0;
    virtual void                  configure_(specs::PopulationSpec) = 0;

    virtual size_t              size_() const               = 0;
    virtual std::vector<Substrate> getAsVector_() const            = 0;
    virtual void                merge_(std::vector<Substrate>) = 0;
    virtual void                pruneLineage_(long)        = 0;
    virtual void                snapShot_(long)             = 0;
    virtual void                flushUnpruned_()            = 0;

    // optional methods
	
    // prohibited methods
    virtual std::string classNameAsString_() const = 0;
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
        std::declval<T &>().merge(std::declval<std::vector<Substrate>>()));
    static_assert(
        utilities::TMP::has_signature<UserPopulation, void, HasMerge>{},
        "UserPopulation does not satisfy 'merge' concept requirement");
    void
        merge_(std::vector<Substrate> v) override
    {
      data_.merge(v);
    }

    template <typename T>
    using HasGetAsVector = decltype(std::declval<T &>().getAsVector());
    static_assert(
        utilities::TMP::has_signature<UserPopulation,
                                      std::vector<Substrate>,
                                      HasGetAsVector>{},
        "UserPopulation does not satisfy 'get_as_vector' concept requirement");
    std::vector<Substrate>
        getAsVector_() const override
    {
      return data_.getAsVector();
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
    using HasFlush = decltype(std::declval<T &>().flushUnpruned());
    static_assert(
        utilities::TMP::has_signature<UserPopulation, void, HasFlush>{},
        "UserPopulation does not satisfy 'flush_unpruned' concept requirement");
    void
        flushUnpruned_() override
    {
      data_.flushUnpruned();
    }

    template <typename T>
    using HasPruneLineage =
        decltype(std::declval<T &>().pruneLineage(std::declval<long>()));
    static_assert(
        utilities::TMP::has_signature<UserPopulation, void, HasPruneLineage>{},
        "UserPopulation does not satisfy 'prune_lineage' concept requirement");
    void
        pruneLineage_(long i) override
    {
      data_.pruneLineage(i);
    }

    template <typename T>
    using HasSnapshot =
        decltype(std::declval<T &>().snapShot(std::declval<long>()));
    static_assert(
        utilities::TMP::has_signature<UserPopulation, void, HasSnapshot>{},
        "UserPopulation does not satisfy 'snapshot' concept requirement");
    void
        snapShot_(long i) override
    {
      data_.snapShot(i);
    }

    template <typename T>
    using HasConf = decltype(
        std::declval<T &>().configure(std::declval<specs::PopulationSpec>()));
    template <typename T>
    using HasPubConf = decltype(std::declval<T &>().publishConfiguration());
    static_assert(
        utilities::TMP::has_signature<UserPopulation, void, HasConf>{} &&
            utilities::TMP::
                has_signature<UserPopulation, specs::PopulationSpec, HasPubConf>{},
        "UserPopulation does not satisfy 'configuration' concept requirement");
    specs::PopulationSpec
        publishConfiguration_() override
    {
      auto ps  = data_.publishConfiguration();
      ps.name_ = autoClassNameAsString<UserPopulation>();
      return ps;
    }
    void
        configure_(specs::PopulationSpec c) override
    {
      data_.configure(c);
    }

    // optional methods

    // prohibited methods
    template <typename T>
    using Nameable = decltype(std::declval<T &>().classNameAsString_());
    static_assert(
        std::negation<utilities::TMP::is_detected<UserPopulation, Nameable>>{},
        "Population class cannot provide classNameAsString_()");
    std::string
        classNameAsString_() const override
    {
      return autoClassNameAsString<UserPopulation>();
    }


    // data
    UserPopulation data_;
  };

  std::unique_ptr<PopulationInterface> self_;
};

}   // namespace concepts
}   // namespace ded
