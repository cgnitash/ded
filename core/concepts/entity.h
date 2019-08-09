
#pragma once

#include "../configuration.h"
#include "../specs/entity_spec.h"
#include "../utilities/tmp.h"
#include "encoding.h"
#include "signal.h"

#include <cassert>
#include <deque>
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

// polymorphic wrapper for types that walk, talk, and quack like Entitys
class Entity
{
public:
  template <typename UserEntity>
  Entity(UserEntity x) : self_(new EntityObject<UserEntity>(std::move(x)))
  {
  }

  Entity(const Entity &x) : data(x.data), self_(x.self_->copy_())
  {
  }
  Entity(Entity &&) noexcept = default;

  Entity &
      operator=(const Entity &x)
  {
    Entity tmp(x);
    *this = std::move(tmp);
    return *this;
  }
  Entity &operator=(Entity &&) noexcept = default;

  // this should not be used except when beginning a specific simulation
  void reset_global_entity_ids() { entity_id_ = 0; }

  // public interface of Entitys - how Entitys can be used

  bool
      operator==(const Entity &e) const
  {
    return get_id() == e.get_id();
  }

  bool
      operator!=(const Entity &e) const
  {
    return !(*this == e);
  }

  bool
      operator<(const Entity &e) const
  {
    return get_id() < e.get_id();
  }

  bool
      operator>(const Entity &e) const
  {
    return e < *this;
  }

  bool
      operator<=(const Entity &e) const
  {
    return !(*this > e);
  }

  bool
      operator>=(const Entity &e) const
  {
    return !(*this < e);
  }

  DataStore data;

  long
      get_id() const
  {
    return self_->get_id_();
  }

  long
      get_ancestor() const
  {
    return self_->get_ancestor_();
  }

  Encoding
      get_encoding() const
  {
    return self_->get_encoding_();
  }

  void
      set_encoding(Encoding e)
  {
    self_->set_encoding_(e);
  }

  Encoding
      parse_encoding(std::string s)
  {
    return self_->parse_encoding_(s);
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

  specs::EntitySpec
      publish_configuration()
  {
    return self_->publish_configuration_();
  }

  void
      tick()
  {
    self_->tick_();
  }

  void
      configure(specs::EntitySpec es)
  {
    self_->configure_(es);
  }

private:
  // interface/ABC for an Entity
  struct EntityInterface
  {
    // provided methods
    virtual ~EntityInterface()                     = default;
    virtual EntityInterface *copy_() const         = 0;
    virtual long             get_id_() const       = 0;
    virtual long             get_ancestor_() const = 0;

    // mandatory methods
    virtual void              mutate_()                     = 0;
    virtual void              reset_()                      = 0;
    virtual void              tick_()                       = 0;
    virtual void              input_(std::string, Signal)   = 0;
    virtual Signal            output_(std::string)          = 0;
    virtual void              configure_(specs::EntitySpec) = 0;
    virtual specs::EntitySpec publish_configuration_()      = 0;

    // optional methods
    virtual Encoding get_encoding_() const        = 0;
    virtual void     set_encoding_(Encoding)      = 0;
    virtual Encoding parse_encoding_(std::string) = 0;

    // prohibited methods
    virtual std::string class_name_as_string_() const = 0;
  };

  template <typename UserEntity>
  struct EntityObject final : EntityInterface
  {

    // provided methods
    EntityObject(UserEntity x)
        : id_(++entity_id_), ancestor_(0), data_(std::move(x))
    {
    }

    EntityInterface *
        copy_() const override
    {
      return new EntityObject(*this);
    }

    long
        get_ancestor_() const override
    {
      return ancestor_;
    }

    long
        get_id_() const override
    {
      return id_;
    }

    // mandatory methods

    template <typename T>
    using HasInput =
        decltype(std::declval<T &>().input(std::declval<std::string>(),
                                           std::declval<Signal>()));
    static_assert(utilities::TMP::has_signature<UserEntity, void, HasInput>{},
                  "UserEntity does not satisfy 'input' concept requirement");
    void
        input_(std::string n, Signal s) override
    {
      data_.input(n, s);
    }

    template <typename T>
    using HasOutput =
        decltype(std::declval<T &>().output(std::declval<std::string>()));
    static_assert(
        utilities::TMP::has_signature<UserEntity, Signal, HasOutput>{},
        "UserEntity does not satisfy 'output' concept requirement");
    Signal
        output_(std::string n) override
    {
      return data_.output(n);
    }

    template <typename T>
    using HasTick = decltype(std::declval<T &>().tick());
    static_assert(utilities::TMP::has_signature<UserEntity, void, HasTick>{},
                  "UserEntity does not satisfy 'tick' concept requirement");
    void
        tick_() override
    {
      data_.tick();
    }

    template <typename T>
    using HasReset = decltype(std::declval<T &>().reset());
    static_assert(utilities::TMP::has_signature<UserEntity, void, HasReset>{},
                  "UserEntity does not satisfy 'reset' concept requirement");
    void
        reset_() override
    {
      data_.reset();
    }

    template <typename T>
    using HasMutate = decltype(std::declval<T &>().mutate());
    static_assert(utilities::TMP::has_signature<UserEntity, void, HasMutate>{},
                  "UserEntity does not satisfy 'mutate' concept requirement");
    void
        mutate_() override
    {
      ancestor_ = id_;
      id_       = ++entity_id_;
      data_.mutate();
    }

    template <typename T>
    using HasConf = decltype(
        std::declval<T &>().configure(std::declval<specs::EntitySpec>()));
    template <typename T>
    using HasPubConf = decltype(std::declval<T &>().publish_configuration());
    static_assert(
        utilities::TMP::has_signature<UserEntity, void, HasConf>{} &&
            utilities::TMP::
                has_signature<UserEntity, specs::EntitySpec, HasPubConf>{},
        "UserEntity does not satisfy 'configuration' concept requirement");
    specs::EntitySpec
        publish_configuration_() override
    {
      auto es  = data_.publish_configuration();
      es.name_ = auto_class_name_as_string<UserEntity>();
      return es;
    }
    void
        configure_(specs::EntitySpec c) override
    {
      data_.configure(c);
    }

    // optional methods
    template <typename T>
    using EncodingGettable = decltype(std::declval<T &>().get_encoding());

    Encoding
        get_encoding_() const override
    {
      if constexpr (utilities::TMP::is_detected<UserEntity, EncodingGettable>{})
      {
        return data_.get_encoding();
      }
      else
      {
        return Encoding{};
      }
    }

    template <typename T>
    using EncodingSettable =
        decltype(std::declval<T &>().set_encoding(std::declval<Encoding>()));

    void
        set_encoding_(Encoding e) override
    {
      if constexpr (utilities::TMP::is_detected<UserEntity, EncodingSettable>{})
      {
        data_.set_encoding(e);
      }
    }

    template <typename T>
    using EncodingParsable = decltype(
        std::declval<T &>().parse_encoding(std::declval<std::string>()));

    Encoding
        parse_encoding_(std::string s) override
    {
      if constexpr (utilities::TMP::is_detected<UserEntity, EncodingParsable>{})
      {
        return data_.parse_encoding(s);
      }
      else
      {
        return Encoding{};
      }
    }

    // prohibited methods
    template <typename T>
    using Nameable = decltype(std::declval<T &>().class_name_as_string());
    static_assert(
        std::negation<utilities::TMP::is_detected<UserEntity, Nameable>>{},
        "Environment class cannot provide class_name_as_string()");
    std::string
        class_name_as_string_() const override
    {
      return auto_class_name_as_string<UserEntity>();
    }

    // data
    long id_;
    long ancestor_;

    UserEntity data_;
  };

  static long                      entity_id_;
  std::unique_ptr<EntityInterface> self_;
};

}   // namespace concepts
}   // namespace ded
