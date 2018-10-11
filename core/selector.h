
#pragma once

#include"member_detection.h"
#include "entity.h"
#include "eval_results.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>

namespace life {

// polymorphic wrapper for types that walk, talk, and quack like selectors 
class selector {
public:
  template <typename UserSelector>
  selector(UserSelector x)
      : self_(new selector_object<UserSelector>(std::move(x))) {}

  selector(const selector &x) : self_(x.self_->copy_()) {}
  selector(selector &&) noexcept = default;

  selector &operator=(const selector &x) {
    selector tmp(x);
    *this = std::move(tmp);
    return *this;
  }
  selector &operator=(selector &&) noexcept = default;

  // public interface of selectors - how selectors can be used
  std::vector<entity> select(eval_results &p) {
    return self_->select_(p);
  }

  configuration publish_configuration() const {
   return  self_->publish_configuration_();
  }

  void configure(configuration con) {
    auto real = publish_configuration();
    validate_subset(con, real);
    merge_into(con, real);
    self_->configure_(con);
  }

private:

  // interface/ABC for an selector
  struct selector_interface  {
    virtual ~selector_interface () = default;
    virtual selector_interface *copy_() const = 0;

    virtual std::vector<entity>
    select_( eval_results&) = 0;
    virtual configuration publish_configuration_()const  = 0;
    virtual void configure_(configuration )= 0;
  };

  // concept to test if method is provided by user
//  template <typename T> using nameable = decltype(std::declval<T&>().name());

  template <typename UserSelector> struct selector_object : selector_interface {
    selector_object(UserSelector x) : data_(std::move(x)) {}
    selector_interface *copy_() const override {
      return new selector_object(*this);
    }

	// mandatory methods
	//
    std::vector<entity>
    select_( eval_results& p) override {
      return data_.select(p);
    }
   configuration publish_configuration_() const override {
      return data_.publish_configuration(); 
    }
    void configure_(configuration c) override { data_.configure(c); }

	

/*
	// optional methods
	//
    std::string name_() const override {
      if constexpr (enhanced_type_traits::is_detected<UserSelector, nameable>{})
        return "selector-name:" + data_.name();
      else
        return " #unnamed selector??? ";
    }
*/

    UserSelector data_;
  };

  std::unique_ptr<selector_interface> self_;
};

std::vector<std::string> selector_list();
life::selector configure_selector(std::string_view, configuration );
life::selector make_selector(std::string_view );
} // namespace life
