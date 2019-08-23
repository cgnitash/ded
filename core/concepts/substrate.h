
#pragma once

#include "../configuration.h"
#include "../specs/substrate_spec.h"
#include "../utilities/tmp.h"
#include "encoding.h"
#include "signal.h"

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

// polymorphic wrapper for Substrates
class Substrate
{
public:
  template <typename UserSubstrate>
  Substrate(UserSubstrate x) : self_(new SubstrateObject<UserSubstrate>(std::move(x)))
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
  void resetGlobalSubstrateIDs() { entity_id_ = 0; }

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
      input(std::string n, Signal s)
  {
    self_->input_(n, s);
  }

  Signal
      output(std::string n)
  {
    return self_->output_(n);
  }

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
    return self_->publishConfiguration_();
  }

  void
      tick()
  {
    self_->tick_();
  }

  void
      configure(specs::SubstrateSpec es)
  {
    self_->configure_(es);
  }

private:
  // interface/ABC for an Substrate
  struct SubstrateInterface
  {
    // provided methods
    virtual ~SubstrateInterface()                     = default;
    virtual SubstrateInterface *copy_() const         = 0;
    virtual long             getID_() const       = 0;
    virtual long             getAncestor_() const = 0;

    // mandatory methods
    virtual void              mutate_()                     = 0;
    virtual void              reset_()                      = 0;
    virtual void              tick_()                       = 0;
    virtual void              input_(std::string, Signal)   = 0;
    virtual Signal            output_(std::string)          = 0;
    virtual void              configure_(specs::SubstrateSpec) = 0;
    virtual specs::SubstrateSpec publishConfiguration_()      = 0;

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
        : id_(++entity_id_), ancestor_(0), data_(std::move(x))
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

    // mandatory methods

    template <typename T>
    using HasInput =
        decltype(std::declval<T &>().input(std::declval<std::string>(),
                                           std::declval<Signal>()));
    static_assert(utilities::TMP::has_signature<UserSubstrate, void, HasInput>{},
                  "UserSubstrate does not satisfy 'input' concept requirement");
    void
        input_(std::string n, Signal s) override
    {
      data_.input(n, s);
    }

    template <typename T>
    using HasOutput =
        decltype(std::declval<T &>().output(std::declval<std::string>()));
    static_assert(
        utilities::TMP::has_signature<UserSubstrate, Signal, HasOutput>{},
        "UserSubstrate does not satisfy 'output' concept requirement");
    Signal
        output_(std::string n) override
    {
      return data_.output(n);
    }

    template <typename T>
    using HasTick = decltype(std::declval<T &>().tick());
    static_assert(utilities::TMP::has_signature<UserSubstrate, void, HasTick>{},
                  "UserSubstrate does not satisfy 'tick' concept requirement");
    void
        tick_() override
    {
      data_.tick();
    }

    template <typename T>
    using HasReset = decltype(std::declval<T &>().reset());
    static_assert(utilities::TMP::has_signature<UserSubstrate, void, HasReset>{},
                  "UserSubstrate does not satisfy 'reset' concept requirement");
    void
        reset_() override
    {
      data_.reset();
    }

    template <typename T>
    using HasMutate = decltype(std::declval<T &>().mutate());
    static_assert(utilities::TMP::has_signature<UserSubstrate, void, HasMutate>{},
                  "UserSubstrate does not satisfy 'mutate' concept requirement");
    void
        mutate_() override
    {
      ancestor_ = id_;
      id_       = ++entity_id_;
      data_.mutate();
    }

    template <typename T>
    using HasConf = decltype(
        std::declval<T &>().configure(std::declval<specs::SubstrateSpec>()));
    template <typename T>
    using HasPubConf = decltype(std::declval<T &>().publishConfiguration());
    static_assert(
        utilities::TMP::has_signature<UserSubstrate, void, HasConf>{} &&
            utilities::TMP::
                has_signature<UserSubstrate, specs::SubstrateSpec, HasPubConf>{},
        "UserSubstrate does not satisfy 'configuration' concept requirement");
    specs::SubstrateSpec
        publishConfiguration_() override
    {
      auto es  = data_.publishConfiguration();
      es.name_ = autoClassNameAsString<UserSubstrate>();
      return es;
    }
    void
        configure_(specs::SubstrateSpec c) override
    {
      data_.configure(c);
    }

    // optional methods
    template <typename T>
    using EncodingGettable = decltype(std::declval<T &>().getEncoding());

    Encoding
        getEncoding_() const override
    {
      if constexpr (utilities::TMP::is_detected<UserSubstrate, EncodingGettable>{})
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
      if constexpr (utilities::TMP::is_detected<UserSubstrate, EncodingSettable>{})
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
      if constexpr (utilities::TMP::is_detected<UserSubstrate, EncodingParsable>{})
      {
        return data_.parseEncoding(s);
      }
      else
      {
        return Encoding{};
      }
    }

    // prohibited methods
    template <typename T>
    using Nameable = decltype(std::declval<T &>().classNameAsString_());
    static_assert(
        std::negation<utilities::TMP::is_detected<UserSubstrate, Nameable>>{},
        "Substrate class cannot provide classNameAsString_()");
    std::string
        classNameAsString_() const override
    {
      return autoClassNameAsString<UserSubstrate>();
    }

    // data
    long id_;
    long ancestor_;

    UserSubstrate data_;
  };

  static long                      entity_id_;
  std::unique_ptr<SubstrateInterface> self_;
};

}   // namespace concepts
}   // namespace ded
