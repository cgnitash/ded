
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
    return self_->publishConfiguration_();
  }

  void
      configure(specs::ConverterSpec es)
  {
    self_->configure_(es);
  }

  Signal
      convert(Signal s)
  {
    return self_->convert_(s);
  }

private:
  // interface/ABC for an Converter
  struct ConverterInterface
  {
  public:
    // provided methods
    virtual ~ConverterInterface()             = default;
    virtual ConverterInterface *copy_() const = 0;

    // mandatory methods
    //virtual std::string fromSignal_() const = 0;
    //virtual std::string toSignal_() const = 0;
    virtual Signal convert_(Signal) = 0;
    virtual void                 configure_(specs::ConverterSpec) = 0;
    virtual specs::ConverterSpec publishConfiguration_()          = 0;
	
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

	// provided global method
    std::string
        classNameAsString_() const override
    {
        return autoClassNameAsString<UserConverter>();
    }

    // mandatory methods
    //
/*	
    template <typename T>
    using HasFromSignal =
        decltype(std::declval<T &>().fromSignal());
    std::string
        fromSignal_() const override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserConverter, std::string, HasFromSignal>{})
        return data_.fromSignal();
      else
        static_assert(ded::utilities::TMP::concept_fail<UserConverter>{},
                      "\033[35mConverter does not satisfy "
                      "\033[33m\"fromSignal\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasToSignal =
        decltype(std::declval<T &>().toSignal());
    std::string
        toSignal_() const override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserConverter, std::string, HasToSignal>{})
        return data_.toSignal();
      else
        static_assert(ded::utilities::TMP::concept_fail<UserConverter>{},
                      "\033[35mConverter does not satisfy "
                      "\033[33m\"toSignal\"\033[35m concept "
                      "requirement\033[0m");
    }
*/
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
    using HasPubConf = decltype(std::declval<T &>().publishConfiguration());
    specs::ConverterSpec
        publishConfiguration_() override
    {
      if constexpr (utilities::TMP::has_signature<UserConverter,
                                                  specs::ConverterSpec,
                                                  HasPubConf>{})
      {
        auto es  = data_.publishConfiguration();
        es.name_ = autoClassNameAsString<UserConverter>();

		using std::placeholders::_1;
		 std::function<concepts::Signal(concepts::Signal)>   f_convert = 
			 std::bind( &UserConverter::convert, data_, _1 );
		es.conversion_ = f_convert; //data_.convert;

        return es;
      }
      else
        static_assert(ded::utilities::TMP::concept_fail<UserConverter>{},
                      "\033[35mConverter does not satisfy "
                      "\033[33m\"configurable\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasConf = decltype(
        std::declval<T &>().configure(std::declval<specs::ConverterSpec>()));
    void
        configure_(specs::ConverterSpec c) override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserConverter, void, HasConf>{})
      {
        data_.configure(c);
      }
      else
        static_assert(ded::utilities::TMP::concept_fail<UserConverter>{},
                      "\033[35mConverter does not satisfy "
                      "\033[33m\"publishable\"\033[35m concept "
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
