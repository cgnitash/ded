
#pragma once

#include"member_detection.h"
#include"configuration.h"
#include"encoding.h"
#include"entity.h"
#include"signal.h"


#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>
#include <map>

namespace life {

// polymorphic wrapper for types that walk, talk, and quack like environments 
class environment {
public:
  template <typename UserEnvironment>
  environment(UserEnvironment x) : self_(new environment_object<UserEnvironment>(std::move(x))) {}

  environment(const environment &x) : self_(x.self_->copy_()) {}
  environment(environment &&) noexcept = default;

  environment &operator=(const environment &x) {
    environment tmp(x);
    *this = std::move(tmp);
    return *this;
  }
  environment &operator=(environment &&) noexcept = default;

  // public interface of environments - how environments can be used
  std::vector<entity> evaluate( std::vector<entity> p) {
    return self_->evaluate_(p);
  }

  configuration publish_configuration() const {
   return  self_->publish_configuration_();
  }


  void configure(configuration con) const {
    auto real = publish_configuration();
    validate_subset(con, real);
    merge_into(con, real);
    self_->configure_(con);
  }

private:

  // interface/ABC for an environment
  struct environment_interface  {
    virtual ~environment_interface () = default;
    virtual environment_interface *copy_() const = 0;

    virtual configuration publish_configuration_() = 0;
    virtual void configure_(configuration ) = 0;

    virtual std::vector<entity> evaluate_(std::vector<entity> ) = 0;
  };

  // concept to test if method is provided by user
//  template <typename T> using nameable = decltype(std::declval<T&>().name());

  template <typename UserEnvironment> struct environment_object final : environment_interface {

    environment_object(UserEnvironment x) : data_(std::move(x)) {}
 
 	environment_interface *copy_() const override {
      return new environment_object(*this);
    }

	// mandatory methods
	//
    std::vector<entity> evaluate_(std::vector<entity> p) override {
      return data_.evaluate(p);
    }

   configuration publish_configuration_() override {
      return data_.publish_configuration(); 
    }

    void configure_(configuration c) override { data_.configure(c); }

/*
	// optional methods
	//
    std::string name_() const override {
      if constexpr (enhanced_type_traits::is_detected<UserEnvironment, nameable>{})
        return "environment-name:" + data_.name();
      else
        return " #unnamed environment??? ";
    }
*/

    UserEnvironment data_;
  };

  std::unique_ptr<environment_interface> self_;
};

std::vector<std::string> environment_list();
environment make_environment(std::string_view);
environment configure_environment(std::string_view, configuration);
} // namespace life
