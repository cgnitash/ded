# pragma once

#include"../../core/configuration.h"
#include"../../core/signal.h"
#include"../../core/encoding.h"
#include"../../core/entity.h"
#include"../../core/utilities.h"

#include <vector>
#include <algorithm>


class linear {

	std::vector<life::entity> pop_;
	std::string entity_name_{"null_entity"};
	life::configuration entity_config_;
	long size_{10};
public:
  void configure(life::configuration con) {
    size_ = con["size"];
    entity_name_ = std::string(con["entity"][0]);
    entity_config_ = con["entity"][1];
	pop_.clear();
    util::rv3::generate_n(util::rv3::back_inserter(pop_), size_, [&] {
      auto org = life::make_entity(entity_name_);
      org.configure(entity_config_);
      return org;
    });
  }
  linear() { configure(publish_configuration()); }
  life::configuration publish_configuration() {
    life::configuration con;
    con["size"] = size_;
    con["entity"] = {entity_name_,{}};
    return con;
  }

  std::vector<life::entity> get_as_vector();
  void merge(std::vector<life::entity>);
  void clear();
  life::configuration get_stats();
};
