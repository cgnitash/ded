
#pragma once

#include "configuration.h"
#include "encoding.h"
#include "entity.h"
#include "enhanced_type_traits.h"
#include "population.h"
#include "signal.h"
#include "specs/environment_spec.h"

#include <cassert>
#include <experimental/filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace life {

// polymorphic wrapper for types that walk, talk, and quack like environments
class environment {
public:
  template <typename UserEnvironment>
  environment(UserEnvironment x)
      : self_(new environment_object<UserEnvironment>(std::move(x)))
  {
  }

  environment(const environment &x) : self_(x.self_->copy_()) {}
  environment(environment &&) noexcept = default;

  environment &operator=(const environment &x)
  {
    environment tmp(x);
    *this = std::move(tmp);
    return *this;
  }
  environment &operator=(environment &&) noexcept = default;

  // public interface of environments - how environments can be used
  population evaluate(population p)
  {
    auto class_name = self_->class_name_as_string_();
    global_path += class_name + "/";
    std::experimental::filesystem::create_directory(global_path);

    auto p_r = self_->evaluate_(p);

    life::global_path = life::global_path.substr(
        0, life::global_path.length() - class_name.length() - 1);
    return p_r;
  }

  environment_spec publish_configuration() const
  {
    return self_->publish_configuration_();
  }

  void configure(environment_spec es) const
  {
	//environment_spec es =  self_->publish_configuration_(), c;
	//c.from_json(con);
	//es.validate_and_merge(c);
	
  //  auto real = publish_configuration();
  //  validate_subset(con, real);
  //  merge_into(con, real);
  //  self_->configure_(con);
    self_->configure_(es);
  }

private:
  // interface/ABC for an environment
  struct environment_interface
  {
    virtual ~environment_interface()             = default;
    virtual environment_interface *copy_() const = 0;

    virtual environment_spec publish_configuration_()  = 0;
    virtual void          configure_(environment_spec) = 0;

    virtual population  evaluate_(population)         = 0;
    virtual std::string class_name_as_string_() const = 0;
  };

  template <typename UserEnvironment>
  struct environment_object final : environment_interface
  {

    environment_object(UserEnvironment x) : data_(std::move(x)) {}

    environment_interface *copy_() const override
    {
      return new environment_object(*this);
    }

    // mandatory methods

    template <typename T>
    using HasEvaluate =
        decltype(std::declval<T &>().evaluate(std::declval<population>()));
    static_assert(
        enhanced_type_traits::
            has_signature<UserEnvironment, population, HasEvaluate>{},
        "UserEnvironment does not satisfy 'evaluate' concept requirement");
    population evaluate_(population p) override { return data_.evaluate(p); }

    template <typename T>
    using HasConf =
        decltype(std::declval<T &>().configure(std::declval<environment_spec>()));
    template <typename T>
    using HasPubConf = decltype(std::declval<T &>().publish_configuration());
    static_assert(
        enhanced_type_traits::has_signature<UserEnvironment, void, HasConf>{} &&
            enhanced_type_traits::
                has_signature<UserEnvironment, environment_spec, HasPubConf>{},
        "UserEnvironment does not satisfy 'configuration' concept requirement");
    environment_spec publish_configuration_() override
    {
      return data_.publish_configuration();
    }
    void configure_(environment_spec c) override { data_.configure(c); }

    // optional methods

    // prohibited methods
    template <typename T>
    using Nameable = decltype(std::declval<T &>().class_name_as_string());
    static_assert(
        std::negation<
            enhanced_type_traits::is_detected<UserEnvironment, Nameable>>{},
        "Environment class cannot provide class_name_as_string()");
    std::string class_name_as_string_() const override
    {
      return auto_class_name_as_string<UserEnvironment>();
    }

    // data

    UserEnvironment data_;
  };

  std::unique_ptr<environment_interface> self_;
};

}   // namespace life
