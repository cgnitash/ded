
#pragma once

#include"member_detection.h"
#include"configuration.h"


#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>
#include <map>

namespace life {

// polymorphic wrapper for types that walk, talk, and quack like organims
class entity {
public:
  template <typename UserEntity>
  entity(UserEntity x) : self_(new entity_object<UserEntity>(std::move(x))) {}

  entity(const entity &x) : self_(x.self_->copy_()) {}
  entity(entity &&) noexcept = default;

  entity &operator=(const entity &x) {
    entity tmp(x);
    *this = std::move(tmp);
    return *this;
  }
  entity &operator=(entity &&) noexcept = default;

  // public interface of entitys - how entitys can be used
  long update() {
   score_ =  self_->update_();
  	return score_;
  }
  void mutate() const { self_->mutate_(); }

  configuration publish_configuration() const {
   return  self_->publish_configuration_();
  }

  double score() const { return score_; }
private:

  // interface/ABC for an entity
  struct entity_interface  {
    virtual ~entity_interface () = default;
    virtual entity_interface *copy_() const = 0;

    virtual void mutate_() = 0;
    virtual configuration publish_configuration_() = 0;
	virtual long update_() = 0;
  };

  // concept to test if method is provided by user
//  template <typename T> using nameable = decltype(std::declval<T&>().name());

  template <typename UserEntity> struct entity_object : entity_interface {
    entity_object(UserEntity x) : data_(std::move(x)) {}
    entity_interface *copy_() const override {
      return new entity_object(*this);
    }

	// mandatory methods
	//
    long update_() override {
      return data_.update(); 
    }

   void mutate_() override {
      data_.mutate(); 
    }
   
   configuration publish_configuration_() override {
      return data_.publish_configuration(); 
    }


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

    UserEntity data_;
  };

  double score_ = 0;
  std::unique_ptr<entity_interface> self_;
};

std::vector<std::string> entity_list();
//static std::vector<std::string> _entity_list;
entity make_entity(std::string_view);
entity configure_entity(std::string_view, configuration);
} // namespace life
