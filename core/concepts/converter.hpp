
#pragma once

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "../configuration.hpp"
#include "signal.hpp"

namespace ded
{
namespace concepts
{

// polymorphic wrapper for Converter
class Converter
{
public:
  template <typename UserConverter>
  Converter(UserConverter x) : self_(new ConverterOject<UserConverter>(std::move(x)))
  {
  }

  Converter(const Converter &x) : self_(x.self_->copy_())
  {
  }

  Converter(Converter &&) noexcept = default;

  Converter &
      operator=(const Converter &x)
  {
    Converter tmp(x);
    *this = std::move(tmp);
    return *this;
  }

  Converter &operator=(Converter &&) noexcept = default;

  specs::ConverterSpec
      publishConfiguration()
  {
    specs::ConverterSpec spec;
    spec.setConfigured(false);
    self_->configuration_(spec);
    spec.setConfigured(true);
    return spec;
  }

  void
      configure(specs::ConverterSpec es)
  {
    self_->configuration_(es);
  }

  Signal
      convert(Signal s)
  {
    return self_->convert_(s);
  }

  specs::ConversionSignature
      getConversionFunction()
  {
    return self_->getConversionFunction_();
  }

private:
  // interface/ABC for an Converter
  struct ConverterInterface
  {
  public:
    // provided methods
    virtual ~ConverterInterface()             = default;
    virtual ConverterInterface *copy_() const = 0;

    virtual specs::ConversionSignature getConversionFunction_() const = 0;

    // mandatory methods
    virtual Signal convert_(Signal) = 0;
    virtual void   configuration_(specs::ConverterSpec &) = 0;
    
	//virtual void configure_(specs::ConverterSpec) = 0;
    //virtual specs::ConverterSpec publishConfiguration_()          = 0;
	
    // optional methods

    // prohibited methods
    virtual std::string classNameAsString_() const = 0;
  };

  template <typename UserConverter>
  struct ConverterOject final : ConverterInterface
  {

  public:
    // provided methods
    //
    ConverterOject(UserConverter x) : data_(std::move(x))
    {
    }

    ConverterInterface *
        copy_() const override
    {
      return new ConverterOject(*this);
    }

    specs::ConversionSignature
        getConversionFunction_() const override
    {
      using std::placeholders::_1;
      return std::bind(&UserConverter::convert, data_, _1);
    }
        // provided global method
    std::string
        classNameAsString_() const override
    {
        return autoClassNameAsString<UserConverter>();
    }

    // mandatory methods
    //
    template <typename T>
    using HasConvert =
        decltype(std::declval<T &>().convert(std::declval<Signal>()));
	Signal
        convert_(Signal s) override
		
    {
      if constexpr (utilities::TMP::
                        has_signature<UserConverter, Signal, HasConvert>{})
        return data_.convert(s);
      else
        static_assert(ded::utilities::TMP::concept_fail<UserConverter>{},
                      "\033[35mConverter does not satisfy "
                      "\033[33m\"convert\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasConf = decltype(
        std::declval<T &>().configuration(std::declval<specs::ConverterSpec&>()));
    template <typename T>
    using HasConf_byval = decltype(
        std::declval<T &>().configuration(std::declval<specs::ConverterSpec>()));
    void
        configuration_(specs::ConverterSpec &c) override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserConverter, void, HasConf>{})
      {
        if constexpr (utilities::TMP::
                          has_signature<UserConverter, void, HasConf_byval>{})
          static_assert(ded::utilities::TMP::concept_fail<UserConverter>{},
                        "\033[35mconfiguration must take ConverterSpec by "
                        "non-const reference"
                        "\033[0m");

        data_.configuration(c);
        c.name_ = autoClassNameAsString<UserConverter>();
	  }
      else
        static_assert(ded::utilities::TMP::concept_fail<UserConverter>{},
                      "\033[35mPopulation does not satisfy "
                      "\033[33m\"configurable\"\033[35m concept "
                      "requirement\033[0m");
    }

    // optional methods

    // prohibited methods

    // data
    UserConverter data_;
  };

  std::unique_ptr<ConverterInterface> self_;

  // wtf? regular string compiles 'maybe' but is wrong
  // std::string user_specified_name_;
  //std::vector<std::string> user_specified_name_;
};

}   // namespace concepts
}   // namespace ded
