
#pragma once

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "../configuration.hpp"
#include "../specs/substrate_spec.hpp"
#include "../utilities/tmp.hpp"
#include "encoding.hpp"
#include "signal.hpp"

namespace ded
{
namespace concepts
{

//concepts::Converter  makeConverter(specs::ConverterSpec);

// polymorphic wrapper for Substrates
class Substrate
{
public:
  template <typename UserSubstrate>
  Substrate(UserSubstrate x)
      : self_(new SubstrateObject<UserSubstrate>(std::move(x)))
  {
  }

  Substrate(const Substrate &x) : data(x.data), self_(x.self_->copy_())
  {
  }

  Substrate(Substrate &&) noexcept = default;

  Substrate &
      operator=(const Substrate &x)
  {
    Substrate tmp(x);
    *this = std::move(tmp);
    return *this;
  }

  Substrate &operator=(Substrate &&) noexcept = default;

  // this should not be used except when beginning a specific simulation
  // ** affects global substrate IDs
  void
      resetGlobalSubstrateIDs()
  {
    substrate_id_ = 0;
  }

  // public interface of Substrates

  bool
      operator==(const Substrate &e) const
  {
    return getID() == e.getID();
  }

  bool
      operator!=(const Substrate &e) const
  {
    return !(*this == e);
  }

  bool
      operator<(const Substrate &e) const
  {
    return getID() < e.getID();
  }

  bool
      operator>(const Substrate &e) const
  {
    return e < *this;
  }

  bool
      operator<=(const Substrate &e) const
  {
    return !(*this > e);
  }

  bool
      operator>=(const Substrate &e) const
  {
    return !(*this < e);
  }

  DataStore data;

  long
      getID() const
  {
    return self_->getID_();
  }

  long
      getAncestor() const
  {
    return self_->getAncestor_();
  }

  Encoding
      getEncoding() const
  {
    return self_->getEncoding_();
  }

  void
      setEncoding(Encoding e)
  {
    self_->setEncoding_(e);
  }

  Encoding
      parseEncoding(std::string s)
  {
    return self_->parseEncoding_(s);
  }

  void
      input( specs::ConversionSignatureSequence sequence, Signal s );

  Signal
      output( specs::ConversionSignatureSequence sequence);

  void
      mutate()
  {
    self_->mutate_();
  }

  void
      reset()
  {
    self_->reset_();
  }

  specs::SubstrateSpec
      publishConfiguration()
  {
  specs::SubstrateSpec spec;
  spec.setConfigured(false);
  self_->configuration_(spec);
  spec.setConfigured(true);
  return spec;
  }

  void
      tick()
  {
    self_->tick_();
  }

  void
      configure(specs::SubstrateSpec es)
  {
    self_->configuration_(es);
  }

private:
  // interface/ABC for an Substrate
  struct SubstrateInterface
  {
    // provided methods
    virtual ~SubstrateInterface()                    = default;
    virtual SubstrateInterface *copy_() const        = 0;
    virtual long                getID_() const       = 0;
    virtual long                getAncestor_() const = 0;

    // mandatory methods
    virtual void                 mutate_()                        = 0;
    virtual void                 reset_()                         = 0;
    virtual void                 tick_()                          = 0;
    virtual void                 input_(std::string, Signal)      = 0;
    virtual Signal               output_(std::string)             = 0;
    virtual void                 configuration_(specs::SubstrateSpec &) = 0;

    // optional methods
    virtual Encoding getEncoding_() const        = 0;
    virtual void     setEncoding_(Encoding)      = 0;
    virtual Encoding parseEncoding_(std::string) = 0;

    // prohibited methods
    virtual std::string classNameAsString_() const = 0;
  };

  template <typename UserSubstrate>
  struct SubstrateObject final : SubstrateInterface
  {

    // provided methods
    SubstrateObject(UserSubstrate x)
        : id_(++substrate_id_), ancestor_(0), data_(std::move(x))
    {
    }

    SubstrateInterface *
        copy_() const override
    {
      return new SubstrateObject(*this);
    }

    long
        getAncestor_() const override
    {
      return ancestor_;
    }

    long
        getID_() const override
    {
      return id_;
    }

	// provided global method
    std::string
        classNameAsString_() const override
    {
        return autoClassNameAsString<UserSubstrate>();
    }

    // mandatory methods
    //

    template <typename T>
    using HasInput =
        decltype(std::declval<T &>().input(std::declval<std::string>(),
                                           std::declval<Signal>()));
    void
        input_(std::string n, Signal s) override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserSubstrate, void, HasInput>{})
        data_.input(n, s);
      else
        static_assert(ded::utilities::TMP::concept_fail<UserSubstrate>{},
                      "\033[35mSubstrate does not satisfy "
                      "\033[33m\"input\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasOutput =
        decltype(std::declval<T &>().output(std::declval<std::string>()));
    Signal
        output_(std::string n) override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserSubstrate, Signal, HasOutput>{})
        return data_.output(n);
      else
        static_assert(ded::utilities::TMP::concept_fail<UserSubstrate>{},
                      "\033[35mSubstrate does not satisfy "
                      "\033[33m\"output\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasTick = decltype(std::declval<T &>().tick());
    void
        tick_() override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserSubstrate, void, HasTick>{})
        data_.tick();
      else
        static_assert(ded::utilities::TMP::concept_fail<UserSubstrate>{},
                      "\033[35mSubstrate does not satisfy "
                      "\033[33m\"tick\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasReset = decltype(std::declval<T &>().reset());
    void
        reset_() override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserSubstrate, void, HasReset>{})
        data_.reset();
      else
        static_assert(ded::utilities::TMP::concept_fail<UserSubstrate>{},
                      "\033[35mSubstrate does not satisfy "
                      "\033[33m\"reset\"\033[35m concept "
                      "requirement\033[0m");
    }

    template <typename T>
    using HasMutate = decltype(std::declval<T &>().mutate());
    void
        mutate_() override
    {
      if constexpr (utilities::TMP::
                        has_signature<UserSubstrate, void, HasMutate>{})
      {
        ancestor_ = id_;
        id_       = ++substrate_id_;
        data_.mutate();
      }
      else
        static_assert(ded::utilities::TMP::concept_fail<UserSubstrate>{},
                      "\033[35mSubstrate does not satisfy "
                      "\033[33m\"mutate\"\033[35m concept "
                      "requirement\033[0m");
    }


    template <typename T>
    using HasConf = decltype(
        std::declval<T &>().configuration(std::declval<specs::SubstrateSpec&>()));
    void
        configuration_(specs::SubstrateSpec &c) override
    {
      if constexpr (utilities::TMP::has_signature<UserSubstrate, void, HasConf>{})
	  {
        data_.configuration(c);
        c.name_ = autoClassNameAsString<UserSubstrate>();
	  }
      else
        static_assert(ded::utilities::TMP::concept_fail<UserSubstrate>{},
                      "\033[35mSubstratedoes not satisfy "
                      "\033[33m\"configurable\"\033[35m concept "
                      "requirement\033[0m");
    }

    // optional methods
    template <typename T>
    using EncodingGettable = decltype(std::declval<T &>().getEncoding());

    Encoding
        getEncoding_() const override
    {
      if constexpr (utilities::TMP::is_detected<UserSubstrate,
                                                EncodingGettable>{})
      {
        return data_.getEncoding();
      }
      else
      {
        return Encoding{};
      }
    }

    template <typename T>
    using EncodingSettable =
        decltype(std::declval<T &>().setEncoding(std::declval<Encoding>()));

    void
        setEncoding_(Encoding e) override
    {
      if constexpr (utilities::TMP::is_detected<UserSubstrate,
                                                EncodingSettable>{})
      {
        data_.setEncoding(e);
      }
    }

    template <typename T>
    using EncodingParsable = decltype(
        std::declval<T &>().parseEncoding(std::declval<std::string>()));

    Encoding
        parseEncoding_(std::string s) override
    {
      if constexpr (utilities::TMP::is_detected<UserSubstrate,
                                                EncodingParsable>{})
      {
        return data_.parseEncoding(s);
      }
      else
      {
        return Encoding{};
      }
    }

    // prohibited methods

    // data
    long id_;
    long ancestor_;

    UserSubstrate data_;
  };

  static long                         substrate_id_;
  std::unique_ptr<SubstrateInterface> self_;
};

}   // namespace concepts
}   // namespace ded
