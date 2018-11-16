
#pragma once

#include"member_detection.h"
#include"configuration.h"
#include"encoding.h"
#include"signal.h"
#include"entity.h"


#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>
#include <map>
#include <functional>

namespace life {


// polymorphic wrapper for types that walk, talk, and quack like organisms
class population {
public:
  template <typename UserEntity>
  population(UserEntity x) : self_(new population_object<UserEntity>(std::move(x))) {}

  population(const population &x) : data(x.data) ,self_(x.self_->copy_())  {}
  population(population &&) noexcept = default;

  population &operator=(const population &x) {
    population tmp(x);
    *this = std::move(tmp);
    return *this;
  }
  population &operator=(population &&) noexcept = default;

  // public interface of populations - how populations can be used
  life::configuration data;

  void merge(std::vector<entity> v) { self_->merge_(v); }

  std::vector<entity> get_as_vector() { return self_->get_as_vector_(); }

  void clear() { self_->clear_(); }

  life::configuration get_stats() { return self_->get_stats_(); }

  configuration publish_configuration() {
   return  self_->publish_configuration_();
  }

  void configure(configuration con) {
    auto real = publish_configuration();
    validate_subset(con, real);
    merge_into(con, real);
    self_->configure_(con);
  }

private:

  // interface/ABC for an population
  struct population_interface  {
    virtual ~population_interface () = default;
    virtual population_interface *copy_() const = 0;

    virtual configuration publish_configuration_() = 0;
    virtual void configure_(configuration ) = 0;

    virtual std::vector<life::entity> get_as_vector_() = 0;
    virtual void merge_(std::vector<life::entity>) = 0;
    virtual void clear_() = 0;
    virtual life::configuration get_stats_() = 0;
  };

  // concept to test if method is provided by user
//  template <typename T> using nameable = decltype(std::declval<T&>().name());

  template <typename UserEntity> struct population_object final : population_interface {

    // provided methods
    population_object(UserEntity x) : data_(std::move(x)) {}

    population_interface *copy_() const override {
      return new population_object(*this);
    }


    // mandatory methods
    //

    void merge_(std::vector<entity> v) override {  data_.merge(v); }

    std::vector<entity> get_as_vector_() override {  return data_.get_as_vector(); }

    void clear_() override { data_.clear(); }

    life::configuration get_stats_() { return data_.get_stats(); }

    configuration publish_configuration_() override {
      return data_.publish_configuration(); 
    }

    void configure_(configuration c) override { data_.configure(c); }

/*
	// optional methods
	//
    std::string name_() const override {
      if constexpr (enhanced_type_traits::is_detected<UserEntity, nameable>{})
        return "entity-name:" + data_.name();
      else
        return " #unnamed entity??? ";
    }
*/
	std::vector<long> common_ancestors_;
    UserEntity data_;
  };

  std::unique_ptr<population_interface> self_;
};

std::vector<std::string> population_list();
population make_population(std::string_view);
population configure_population(std::string_view, configuration);
} // namespace life
