
#pragma once

#include "configuration.h"
#include "encoding.h"
#include "entity.h"
#include "member_detection.h"
#include "population.h"
#include "signal.h"

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include <experimental/filesystem>

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
    /*
{
auto pcheck  = p;
auto current = publish_configuration();
for (auto &o : pcheck.get_as_vector())
  if (o.data.size() != current["pre-tags"].size())
  {
    std::cout << "prewtf";
    std::exit(1);
  }
}
    */
    auto p_r = self_->evaluate_(p);
    /*
{
auto pcheck  = p_r;
auto current = publish_configuration();
for (auto &o : pcheck.get_as_vector())
  if (o.data.size() != current["post-tags"].size())
  {
    std::cout << "postwtf";
    std::exit(1);
  }
}
    */
    life::global_path = life::global_path.substr(
        0,
        life::global_path.length() - class_name.length() - 1);
    return p_r;
  }

  configuration publish_configuration() const
  {
    return self_->publish_configuration_();
  }

  void configure(configuration con) const
  {
    auto real = publish_configuration();
    validate_subset(con, real);
    merge_into(con, real);
    self_->configure_(con);
  }

private:
  // interface/ABC for an environment
  struct environment_interface
  {
    virtual ~environment_interface()             = default;
    virtual environment_interface *copy_() const = 0;

    virtual configuration publish_configuration_()  = 0;
    virtual void          configure_(configuration) = 0;

    virtual population evaluate_(population) = 0;
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
    //
    population evaluate_(population p) override { return data_.evaluate(p); }

    configuration publish_configuration_() override
    {
      return data_.publish_configuration();
    }

    void configure_(configuration c) override { data_.configure(c); }

    // optional methods
	//
    // prohibited methods
    template <typename T>
    using Nameable = decltype(std::declval<T &>().class_name_as_string());

    std::string class_name_as_string_() const override
    {
      if constexpr (enhanced_type_traits::is_detected<UserEnvironment,
                                                      Nameable>{})
      {
        static_assert(
            enhanced_type_traits::is_detected<UserEnvironment, Nameable>{},
            "Environment class cannot provide class_name_as_string()");
      } else
        return auto_class_name_as_string<UserEnvironment>();
    }

    // data

    UserEnvironment data_;
  };

  std::unique_ptr<environment_interface> self_;
};

}   // namespace life
