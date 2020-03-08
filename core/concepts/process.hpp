
#pragma once

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "../configuration.hpp"
#include "../specs/process_spec.hpp"
#include "../utilities/tmp.hpp"
#include "encoding.hpp"
#include "population.hpp"
#include "signal.hpp"
#include "substrate.hpp"

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

  specs::ProcessSpec publishConfiguration() const;

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
    virtual void               configuration_(specs::ProcessSpec&) = 0;
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

	// provided global method
    std::string
        classNameAsString_() const override
    {
        return autoClassNameAsString<UserProcess>();
    }

    //
    // mandatory methods
	
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
        static_assert(ded::utilities::TMP::concept_fail<UserProcess>{},
                      "\033[35mProcess does not satisfy "
                      "\033[33m\"evaluate\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasConf = decltype(
        std::declval<T &>().configuration(std::declval<specs::ProcessSpec&>()));
    void
        configuration_(specs::ProcessSpec &c) override
    {
      if constexpr (utilities::TMP::has_signature<UserProcess, void, HasConf>{})
	  {
        data_.configuration(c);
        c.name_ = autoClassNameAsString<UserProcess>();
	  }
      else
        static_assert(ded::utilities::TMP::concept_fail<UserProcess>{},
                      "\033[35mProcess does not satisfy "
                      "\033[33m\"configurable\"\033[35m concept "
                      "requirement\033[0m");
    }

    // optional methods

    // prohibited methods

    // data
    UserProcess data_;
  };

  std::unique_ptr<ProcessInterface> self_;

  void recordTraces(const Population &, std::vector<specs::Trace>);

  specs::TraceConfig traces_;

  int invocations_ = 0;

  specs::ConversionSignatureSequence tag_conversions_;

  concepts::Population applyTagConversions(concepts::Population);

  // wtf? regular string compiles 'maybe' but is wrong
  // std::string user_specified_name_;
  std::vector<std::string> user_specified_name_;
};

}   // namespace concepts
}   // namespace ded
