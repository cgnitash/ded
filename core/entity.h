
#pragma once

#include "configuration.h"
#include "encoding.h"
#include "member_detection.h"
#include "signal.h"

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include <deque>

namespace life {

// polymorphic wrapper for types that walk, talk, and quack like organisms
class entity {
public:
  template <typename UserEntity>
  entity(UserEntity x) : self_(new entity_object<UserEntity>(std::move(x))) {}

  entity(const entity &x) : data(x.data), self_(x.self_->copy_()) {}
  entity(entity &&) noexcept = default;

  entity &operator=(const entity &x) {
    entity tmp(x);
    *this = std::move(tmp);
    return *this;
  }
  entity &operator=(entity &&) noexcept = default;

  // public interface of entitys - how entitys can be used

  bool operator==(const entity &e) const { return get_id() == e.get_id(); }

  bool operator!=(const entity &e) const { return get_id() != e.get_id(); }

  bool operator<(const entity &e) const { return get_id() < e.get_id(); }

  bool operator>(const entity &e) const { return get_id() > e.get_id(); }

  bool operator<=(const entity &e) const { return get_id() <= e.get_id(); }

  bool operator>=(const entity &e) const { return get_id() >= e.get_id(); }

  life::configuration data;

  long get_id() const { return self_->get_id_(); }

  long get_ancestor() const { return self_->get_ancestor_(); }

  //void prune_ancestors(long n) { self_->prune_ancestors_(n); }

  encoding get_encoding() const { return self_->get_encoding_(); }

  void set_encoding(encoding e)  { self_->set_encoding_(e); }


  void input(signal s) { self_->input_(s); }

  signal output() { return self_->output_(); }

  void mutate() { self_->mutate_(); }

  configuration publish_configuration() {
    return self_->publish_configuration_();
  }

  void tick() { self_->tick_(); }

  void configure(configuration con) {
    auto real = publish_configuration();
    validate_subset(con, real);
    merge_into(con, real);
    self_->configure_(con);
  }

private:
  // interface/ABC for an entity
  struct entity_interface {
    virtual ~entity_interface() = default;
    virtual entity_interface *copy_() const = 0;

    virtual long get_id_() const = 0;
  //  virtual void prune_ancestors_(long) = 0;
    virtual long get_ancestor_() const = 0;
  virtual encoding get_encoding_() const =0;

  virtual void set_encoding_(encoding) = 0; 

    virtual void mutate_() = 0;
    virtual configuration publish_configuration_() = 0;
    virtual void tick_() = 0;
    virtual void input_(signal) = 0;
    virtual signal output_() = 0;
    virtual void configure_(configuration) = 0;
  };

  // concept to test if method is provided by user
  //  template <typename T> using nameable =
  //  decltype(std::declval<T&>().name());

  template <typename UserEntity> struct entity_object final : entity_interface {

    // provided methods
    entity_object(UserEntity x)
        : id_(++entity_id_), ancestor_(0), data_(std::move(x)) {}

    entity_interface *copy_() const override {
      return new entity_object(*this);
    }

    // mandatory methods
    //
    void input_(signal s) override { data_.input(s); }
    signal output_() override { return data_.output(); }
    void tick_() override { data_.tick(); }

    long get_ancestor_() const override { return ancestor_; }
    long get_id_() const override { return id_; }


    void mutate_() override {
      ancestor_ = id_;
      id_ = ++entity_id_;
      data_.mutate();
    }

    configuration publish_configuration_() override {
      return data_.publish_configuration();
    }

    void configure_(configuration c) override { data_.configure(c); }

	/*
    void prune_ancestors_(long n) override {
      //    ancestors_.erase(std::begin(ancestors_), std::begin(ancestors_) +
      //    n);
      util::repeat(n, [&] { ancestors_.pop_front(); });
    }
	*/

    // optional methods
    template <typename T>
    using EncodingGettable = decltype(std::declval<T &>().get_encoding());

    template <typename T>
    using EncodingSettable = decltype(std::declval<T &>().set_encoding());

    encoding get_encoding_() const override {
      if constexpr (enhanced_type_traits::is_detected<UserEntity,
                                                      EncodingGettable>{})
        return data_.get_encoding();
      else
        return encoding{};
    }

    void set_encoding_(encoding e) override {
      if constexpr (enhanced_type_traits::is_detected<UserEntity,
                                                      EncodingSettable>{})
        data_.set_encoding(e);
    }
    /*
            // optional methods
            //
        std::string name_() const override {
          if constexpr (enhanced_type_traits::is_detected<UserEntity,
       nameable>{}) 
	   return "entity-name:" + data_.name(); else return " #unnamed
       entity??? ";
        }
    */
    long id_;
    long ancestor_;
    UserEntity data_;
  };

  static long entity_id_;
  std::unique_ptr<entity_interface> self_;
};

std::vector<std::string> entity_list();
entity make_entity(std::string_view);
entity configure_entity(std::string_view, configuration);
} // namespace life
