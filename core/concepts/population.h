
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
#include "signal.h"
#include "substrate.h"

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
    virtual specs::PopulationSpec publishConfiguration_()           = 0;
    virtual void                  configure_(specs::PopulationSpec) = 0;

    virtual size_t                 size_() const                  = 0;
    virtual std::vector<Substrate> getAsVector_() const           = 0;
    virtual void                   merge_(std::vector<Substrate>) = 0;
    virtual void                   pruneLineage_(long)            = 0;
    virtual void                   snapShot_(long)                = 0;
    virtual void                   flushUnpruned_()               = 0;

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
    // dependent template to allow for static_assert error messages
    template <typename = void>
    struct concept_fail : std::false_type
    {
    };

    template <typename T>
    using HasMerge = decltype(
        std::declval<T &>().merge(std::declval<std::vector<Substrate>>()));
    void
        merge_(std::vector<Substrate> v) override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserPopulation, void, HasMerge>{})
        data_.merge(v);
      else
        static_assert(concept_fail{},
                      "\033[35mPopulation does not satisfy "
                      "\033[33m\"merge\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasGetAsVector = decltype(std::declval<T &>().getAsVector());
    std::vector<Substrate>
        getAsVector_() const override
    {
      if constexpr (utilities::TMP::has_signature<UserPopulation,
                                                  std::vector<Substrate>,
                                                  HasGetAsVector>{})
        return data_.getAsVector();
      else
        static_assert(concept_fail{},
                      "\033[35mPopulation does not satisfy "
                      "\033[33m\"get-as-vector\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasSize = decltype(std::declval<T &>().size());
    size_t
        size_() const override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserPopulation, size_t, HasSize>{})
        return data_.size();
      else
        static_assert(concept_fail{},
                      "\033[35mPopulation does not satisfy "
                      "\033[33m\"sized\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasFlush = decltype(std::declval<T &>().flushUnpruned());
    void
        flushUnpruned_() override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserPopulation, void, HasFlush>{})
        data_.flushUnpruned();
      else
        static_assert(concept_fail{},
                      "\033[35mPopulation does not satisfy "
                      "\033[33m\"flushable\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasPruneLineage =
        decltype(std::declval<T &>().pruneLineage(std::declval<long>()));
    void
        pruneLineage_(long i) override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserPopulation, void, HasPruneLineage>{})
        data_.pruneLineage(i);
      else
        static_assert(concept_fail{},
                      "\033[35mPopulation does not satisfy "
                      "\033[33m\"lineage-prune\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasSnapshot =
        decltype(std::declval<T &>().snapShot(std::declval<long>()));
    void
        snapShot_(long i) override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserPopulation, void, HasSnapshot>{})
        data_.snapShot(i);
      else
        static_assert(concept_fail{},
                      "\033[35mPopulation does not satisfy "
                      "\033[33m\"snap-shot\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasPubConf = decltype(std::declval<T &>().publishConfiguration());
    specs::PopulationSpec
        publishConfiguration_() override
    {
      if constexpr (utilities::TMP::has_signature<UserPopulation,
                                                  specs::PopulationSpec,
                                                  HasPubConf>{})
      {
        auto ps  = data_.publishConfiguration();
        ps.name_ = autoClassNameAsString<UserPopulation>();
        return ps;
      }
      else
        static_assert(concept_fail{},
                      "\033[35mPopulation does not satisfy "
                      "\033[33m\"publishable\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasConf = decltype(
        std::declval<T &>().configure(std::declval<specs::PopulationSpec>()));
    void
        configure_(specs::PopulationSpec c) override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserPopulation, void, HasConf>{})
        data_.configure(c);
      else
        static_assert(concept_fail{},
                      "\033[35mPopulation does not satisfy "
                      "\033[33m\"configurable\"\033[35m concept "
                      "requirement\033[0m");
    }

    // optional methods

    // prohibited methods
    template <typename T>
    using Nameable = decltype(std::declval<T &>().classNameAsString_());
    std::string
        classNameAsString_() const override
    {
      if constexpr (utilities::TMP::is_detected<UserPopulation, Nameable>{})
        static_assert(concept_fail{},
                      "Population class cannot provide classNameAsString_()");
      else
        return autoClassNameAsString<UserPopulation>();
    }

    // data
    UserPopulation data_;
  };

  std::unique_ptr<PopulationInterface> self_;
};

}   // namespace concepts
}   // namespace ded
