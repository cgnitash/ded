
#pragma once

#include "configuration.h"
#include "member_detection.h"

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace life {

// polymorphic wrapper for types that walk, talk, and quack like experiments
class experiment {
public:
  template <typename UserExperiment>
  experiment(UserExperiment x)
      : self_(new experiment_object<UserExperiment>(std::move(x))) {}

  experiment(const experiment &x) : self_(x.self_->copy_()) {}
  experiment(experiment &&) noexcept = default;

  experiment &operator=(const experiment &x) {
    experiment tmp(x);
    *this = std::move(tmp);
    return *this;
  }
  experiment &operator=(experiment &&) noexcept = default;

  // public interface of experiments - how experiments can be used
  void run() const { self_->run_(); }

  configuration publish_configuration() const {
    return self_->publish_configuration_();
  }

  void configure(configuration con) const {
    auto real = publish_configuration();
    validate_subset(con, real);
    merge_into(con, real);
    self_->configure_(con);
  }

private:
  // interface/ABC for an experiment
  struct experiment_interface {
    virtual ~experiment_interface() = default;
    virtual experiment_interface *copy_() const = 0;

    virtual void run_() = 0;
    virtual configuration publish_configuration_() = 0;
    virtual void configure_(configuration) = 0;
  };

  template <typename UserExperiment>
  struct experiment_object : experiment_interface {
    experiment_object(UserExperiment x) : data_(std::move(x)) {}
    experiment_interface *copy_() const override {
      return new experiment_object(*this);
    }

    // mandatory methods
    //
    void run_() override { data_.run(); }
    configuration publish_configuration_() override {
      return data_.publish_configuration();
    }
    virtual void configure_(configuration c) override { data_.configure(c); }

    // optional methods

    // data
    UserExperiment data_;
  };

  std::unique_ptr<experiment_interface> self_;
};

experiment make_experiment(std::string_view);
experiment configure_experiment(std::string_view, configuration);
std::vector<std::string> experiment_list();
} // namespace life
