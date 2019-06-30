
#pragma once

#include "../configuration.h"
#include "../specs/environment_spec.h"
#include "../utilities/tmp.h"
#include "encoding.h"
#include "entity.h"
#include "population.h"
#include "signal.h"

#include <cassert>
#include <experimental/filesystem>
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

// polymorphic wrapper for types that walk, talk, and quack like environments
class Environment
{
public:
  template <typename UserEnvironment>
  Environment(UserEnvironment x)
      : self_(new EnvironmentOject<UserEnvironment>(std::move(x)))
  {
  }

  Environment(const Environment &x) : self_(x.self_->copy_())
  {
  }
  Environment(Environment &&) noexcept = default;

  Environment &
      operator=(const Environment &x)
  {
    Environment tmp(x);
    *this = std::move(tmp);
    return *this;
  }
  Environment &operator=(Environment &&) noexcept = default;

  Population evaluate(Population);

  specs::EnvironmentSpec
      publish_configuration() const
  {
    return self_->publish_configuration_();
  }

  void
      configure(specs::EnvironmentSpec es)
  {
    traces_ = es.traces();

    user_specified_name_ = { es.get_user_specified_name() };

    self_->configure_(es);
  }

private:
  // interface/ABC for an Environment
  struct EnvironmentInterface
  {
  public:
    // provided methods
    virtual ~EnvironmentInterface()             = default;
    virtual EnvironmentInterface *copy_() const = 0;

    // mandatory methods
    virtual specs::EnvironmentSpec publish_configuration_()           = 0;
    virtual void                   configure_(specs::EnvironmentSpec) = 0;
    virtual Population             evaluate_(Population)              = 0;

    // optional methods

    // prohibited methods
    virtual std::string class_name_as_string_() const = 0;
  };

  template <typename UserEnvironment>
  struct EnvironmentOject final : EnvironmentInterface
  {

  public:
    // provided methods
    //
    EnvironmentOject(UserEnvironment x) : data_(std::move(x))
    {
    }

    EnvironmentInterface *
        copy_() const override
    {
      return new EnvironmentOject(*this);
    }

    // mandatory methods

    template <typename T>
    using HasEvaluate =
        decltype(std::declval<T &>().evaluate(std::declval<Population>()));
    static_assert(
        utilities::TMP::
            has_signature<UserEnvironment, Population, HasEvaluate>{},
        "UserEnvironment does not satisfy 'evaluate' concept requirement");
    Population
        evaluate_(Population p) override
    {
      return data_.evaluate(p);
    }

    template <typename T>
    using HasConf = decltype(
        std::declval<T &>().configure(std::declval<specs::EnvironmentSpec>()));
    template <typename T>
    using HasPubConf = decltype(std::declval<T &>().publish_configuration());
    static_assert(
        utilities::TMP::has_signature<UserEnvironment, void, HasConf>{} &&
            utilities::TMP::has_signature<UserEnvironment,
                                          specs::EnvironmentSpec,
                                          HasPubConf>{},
        "UserEnvironment does not satisfy 'configuration' concept requirement");
    specs::EnvironmentSpec
        publish_configuration_() override
    {
      auto es  = data_.publish_configuration();
      es.name_ = auto_class_name_as_string<UserEnvironment>();
      return es;
    }
    void
        configure_(specs::EnvironmentSpec c) override
    {
      data_.configure(c);
    }

    // optional methods

    // prohibited methods
    template <typename T>
    using Nameable = decltype(std::declval<T &>().class_name_as_string());
    static_assert(
        std::negation<utilities::TMP::is_detected<UserEnvironment, Nameable>>{},
        "Environment class cannot provide class_name_as_string()");
    std::string
        class_name_as_string_() const override
    {
      return auto_class_name_as_string<UserEnvironment>();
    }

    // data
    UserEnvironment data_;
  };

  std::unique_ptr<EnvironmentInterface> self_;

  void
      record_traces(const Population &pop, std::vector<specs::Trace> ts)
  {

    for (auto n : ts)
      if (invocations_ && !(invocations_ % n.frequency_))
      {
        specs::SignalSpec s_spec{ n.signal_ };
        std::ofstream pop_stats_file{ ded::global_path + s_spec.user_name() +
                                      "_" + std::to_string(invocations_) +
                                      ".csv" };
        pop_stats_file << "id," << s_spec.user_name() << "\n";
        for (const auto &org : pop.get_as_vector())
          pop_stats_file << org.get_id() << ","
                         << std::get<double>(
                                org.data.get_value(s_spec.identifier()))
                         << std::endl;
      }
  }

  specs::TraceConfig traces_;

  int invocations_ = 0;

  // wtf? regular string compiles 'maybe' but is wrong
  // std::string user_specified_name_;
  std::vector<std::string> user_specified_name_;
};

}   // namespace concepts
}   // namespace ded
