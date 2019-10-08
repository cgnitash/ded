
#pragma once

#include "../configuration.h"
#include "../specs/process_spec.h"
#include "../utilities/tmp.h"
#include "encoding.h"
#include "population.h"
#include "signal.h"
#include "substrate.h"

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace ded
{
namespace concepts
{

// polymorphic wrapper for Process
class Process
{
public:
  template <typename UserProcess>
  Process(UserProcess x) : self_(new ProcessOject<UserProcess>(std::move(x)))
  {
  }

  Process(const Process &x) : self_(x.self_->copy_())
  {
  }

  Process(Process &&) noexcept = default;

  Process &
      operator=(const Process &x)
  {
    Process tmp(x);
    *this = std::move(tmp);
    return *this;
  }

  Process &operator=(Process &&) noexcept = default;

  Population evaluate(Population);

  specs::ProcessSpec
      publishConfiguration() const
  {
    return self_->publishConfiguration_();
  }

  void configure(specs::ProcessSpec);

private:
  // interface/ABC for an Process
  struct ProcessInterface
  {
  public:
    // provided methods
    virtual ~ProcessInterface()             = default;
    virtual ProcessInterface *copy_() const = 0;

    // mandatory methods
    virtual specs::ProcessSpec publishConfiguration_()        = 0;
    virtual void               configure_(specs::ProcessSpec) = 0;
    virtual Population         evaluate_(Population)          = 0;

    // optional methods

    // prohibited methods
    virtual std::string classNameAsString_() const = 0;
  };

  template <typename UserProcess>
  struct ProcessOject final : ProcessInterface
  {

  public:
    // provided methods
    //
    ProcessOject(UserProcess x) : data_(std::move(x))
    {
    }

    ProcessInterface *
        copy_() const override
    {
      return new ProcessOject(*this);
    }

    // mandatory methods
    //
    // dependent template to allow for static_assert error messages
    template <typename = void>
    struct concept_fail : std::false_type
    {
    };

    template <typename T>
    using HasEvaluate =
        decltype(std::declval<T &>().evaluate(std::declval<Population>()));
    Population
        evaluate_(Population p) override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserProcess, Population, HasEvaluate>{})
        return data_.evaluate(p);
      else
        static_assert(concept_fail{},
                      "\033[35mSubstrate does not satisfy "
                      "\033[33m\"evaluate\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasPubConf = decltype(std::declval<T &>().publishConfiguration());
    specs::ProcessSpec
        publishConfiguration_() override
    {
      if constexpr (utilities::TMP::has_signature<UserProcess,
                                                  specs::ProcessSpec,
                                                  HasPubConf>{})
      {
        auto es  = data_.publishConfiguration();
        es.name_ = autoClassNameAsString<UserProcess>();
        return es;
      }
      else
        static_assert(concept_fail{},
                      "\033[35mSubstrate does not satisfy "
                      "\033[33m\"publishable\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasConf = decltype(
        std::declval<T &>().configure(std::declval<specs::ProcessSpec>()));
    void
        configure_(specs::ProcessSpec c) override
    {
      if constexpr (utilities::TMP::has_signature<UserProcess, void, HasConf>{})
        data_.configure(c);
      else
        static_assert(concept_fail{},
                      "\033[35mSubstrate does not satisfy "
                      "\033[33m\"configurable\"\033[35m concept "
                      "requirement\033[0m");
    }

    // optional methods

    // prohibited methods
    template <typename T>
    using Nameable = decltype(std::declval<T &>().classNameAsString_());
    static_assert(
        std::negation<utilities::TMP::is_detected<UserProcess, Nameable>>{},
        "Process class cannot provide classNameAsString_()");
    std::string
        classNameAsString_() const override
    {
      return autoClassNameAsString<UserProcess>();
    }

    // data
    UserProcess data_;
  };

  std::unique_ptr<ProcessInterface> self_;

  void recordTraces(const Population &, std::vector<specs::Trace>);

  specs::TraceConfig traces_;

  int invocations_ = 0;

  // wtf? regular string compiles 'maybe' but is wrong
  // std::string user_specified_name_;
  std::vector<std::string> user_specified_name_;
};

}   // namespace concepts
}   // namespace ded
