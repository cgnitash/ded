
#pragma once

#include "configuration.h"
#include "encoding.h"
#include "member_detection.h"
#include "signal.h"

#include <cassert>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

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

  struct data_store {

    std::map<std::string, signal> m;
    signal get_value(std::string n) const { return m.at(n); }
    auto set_value(std::string n, signal v) { m[n] = v; }
	size_t size() const { return m.size(); }
	void clear(std::string i) { m.erase(i); }

  };
  data_store data;

  long get_id() const { return self_->get_id_(); }

  long get_ancestor() const { return self_->get_ancestor_(); }

  encoding get_encoding() const { return self_->get_encoding_(); }

  void set_encoding(encoding e) { self_->set_encoding_(e); }

  encoding parse_encoding(std::string s) { return self_->parse_encoding_(s); }

  void input(std::string n, signal s) { self_->input_(n,s); }

  signal output(std::string n) { return self_->output_(n); }

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
    virtual long get_ancestor_() const = 0;
    virtual encoding get_encoding_() const = 0;
    virtual void set_encoding_(encoding) = 0;
    virtual encoding parse_encoding_(std::string) = 0;

    virtual void mutate_() = 0;
    virtual configuration publish_configuration_() = 0;
    virtual void tick_() = 0;
    virtual void input_(std::string, signal) = 0;
    virtual signal output_(std::string) = 0;
    virtual void configure_(configuration) = 0;
  };

  template <typename UserEntity> struct entity_object final : entity_interface {

    // provided methods
    entity_object(UserEntity x)
        : id_(++entity_id_), ancestor_(0), data_(std::move(x)) {}

    entity_interface *copy_() const override {
      return new entity_object(*this);
    }
    long get_ancestor_() const override { return ancestor_; }
    long get_id_() const override { return id_; }

    // mandatory methods

    void input_(std::string n, signal s) override { data_.input(n, s); }

    signal output_(std::string n) override { return data_.output(n); }

    void tick_() override { data_.tick(); }

    void mutate_() override {
      ancestor_ = id_;
      id_ = ++entity_id_;
      data_.mutate();
    }

    configuration publish_configuration_() override {
      return data_.publish_configuration();
    }

    void configure_(configuration c) override { data_.configure(c); }

    // optional methods
    template <typename T>
    using EncodingGettable = decltype(std::declval<T &>().get_encoding());

    encoding get_encoding_() const override {
      if constexpr (enhanced_type_traits::is_detected<UserEntity,
                                                      EncodingGettable>{}) {
		//std::cout << "in entity get_encoding\n";
        return data_.get_encoding();
	  }
      else
		//std::cout << "no entity get_encoding\n";
        return encoding{};
    }

    template <typename T>
    using EncodingSettable =
        decltype(std::declval<T &>().set_encoding(std::declval<encoding>()));

    void set_encoding_(encoding e) override {
      if constexpr (enhanced_type_traits::is_detected<UserEntity,
                                                      EncodingSettable>{}) {
		//std::cout << "in entity set_encoding\n";
        data_.set_encoding(e);
	  }
	  else {
		  //std::cout << "no entity set_encoding\n";
	  }
    }

    template <typename T>
    using EncodingParsable =
        decltype(std::declval<T &>().parse_encoding(std::declval<std::string>()));

    encoding parse_encoding_(std::string s) override {
      if constexpr (enhanced_type_traits::is_detected<UserEntity,
                                                      EncodingParsable>{}) {
		//std::cout << "in entity parse_encoding\n";
        return data_.parse_encoding(s);
	  }
      else {
		  //std::cout << "no entity parse_encoding\n";
        return encoding{};
	  }
    }

    // data
    long id_;
    long ancestor_;

    UserEntity data_;
  };

  static long entity_id_;
  std::unique_ptr<entity_interface> self_;
};

} // namespace life
