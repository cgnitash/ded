
#pragma once

#include "configuration.h"
#include "encoding.h"
#include "enhanced_type_traits.h"
#include "entity.h"
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
  /*
  std::string get_user_specified_name() const
  {
    return self_->get_user_specified_name_();
  }

  void set_user_specified_name(std::string name)
  {
    self_->set_user_specified_name_(name);
  }
  */

  population evaluate(population p)
  {
    if (!user_specified_name_.empty())
      global_path += user_specified_name_[0] + "_";

    auto class_name = self_->class_name_as_string_();
    global_path += class_name + "/";
    std::experimental::filesystem::create_directory(global_path);

    for (auto n : traces_.pre_)
      if (invocations_ && !(invocations_ % n.frequency_))
      {
        std::ofstream pop_stats_file{ life::global_path +
                                      n.name_ + "_" +
                                      std::to_string(invocations_) + ".csv" };
        pop_stats_file << "id," << n.name_ << "\n";
        for (const auto &org : p.get_as_vector())
          pop_stats_file << org.get_id() << ","
                         << std::get<double>(
                                org.data.get_value(n.identifier_))
                         << std::endl;
        // p.record(n.trace_);
      }

    auto p_r = self_->evaluate_(p);

    for (auto n : traces_.post_)
      if (invocations_ && !(invocations_ % n.frequency_))
      {
        std::ofstream pop_stats_file{ life::global_path +
                                      n.name_ + "_" +
                                      std::to_string(invocations_) + ".csv" };
        pop_stats_file << "id," << n.name_ << "\n";
        for (const auto &org : p_r.get_as_vector())
          pop_stats_file << org.get_id() << ","
                         << std::get<double>(
                                org.data.get_value(n.identifier_))
                         << std::endl;
        // p.record(n.trace_);
      }

    life::global_path.pop_back();
    life::global_path = life::global_path.substr(
        // 0, life::global_path.length() - class_name.length() - 1);
        0,
        life::global_path.find_last_of('/') + 1);

    invocations_++;

    return p_r;
  }

  environment_spec publish_configuration() const
  {
    return self_->publish_configuration_();
  }

  void configure(environment_spec es)
  {
    traces_ = es.traces();

    user_specified_name_ = { es.get_user_specified_name() };

    self_->configure_(es);
  }

private:
  // interface/ABC for an environment
  struct environment_interface
  {
  public:
    // provided methods
    virtual ~environment_interface()             = default;
    virtual environment_interface *copy_() const = 0;
    // virtual std::string     get_user_specified_name_() const              =
    // 0; virtual void     set_user_specified_name_(std::string) = 0;

    // mandatory methods
    virtual environment_spec publish_configuration_()     = 0;
    virtual void             configure_(environment_spec) = 0;
    virtual population       evaluate_(population)        = 0;

    // optional methods

    // prohibited methods
    virtual std::string class_name_as_string_() const = 0;
  };

  template <typename UserEnvironment>
  struct environment_object final : environment_interface
  {

  public:
    // provided methods
    environment_object(UserEnvironment x) : data_(std::move(x)) {}

    environment_interface *copy_() const override
    {
      return new environment_object(*this);
    }
    /*
        std::string get_user_specified_name_() const override
        {
          return user_specified_name_;
        }
        virtual void     set_user_specified_name_(std::string name){
                    user_specified_name_ = name;
            }
    */
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
    using HasConf = decltype(
        std::declval<T &>().configure(std::declval<environment_spec>()));
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

  trace_config traces_;

  int invocations_ = 0;

  // wtf? regular string compiles 'maybe' but is wrong
  // std::string user_specified_name_;
  std::vector<std::string> user_specified_name_;

  std::unique_ptr<environment_interface> self_;
};

}   // namespace life
