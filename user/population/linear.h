#pragma once

#include "../../components.h"

#include <algorithm>
#include <set>
#include <vector>
#include <unordered_map>
#include <utility>


class linear {

  // count of descendants per entity
  std::vector<std::pair<life::entity,long>> fossils_;

  bool track_lineage_{false};
  std::vector<life::entity> pop_;
  std::string entity_name_{"null_entity"};
  //std::string dir_name_{"./"};
  std::string load_spec_{""};
  life::configuration entity_config_;
  long size_{0};

  void update_tree(long p, int count);
  bool found_in_fossils(long) const;
  bool found_in_pop(long) const;
  void initialize();

public:
  linear() { configure(publish_configuration()); }

  void configure(life::configuration con) {
    size_ = con["size"];
    //dir_name_ = con["DIR"];
    load_spec_ = con["load-from"];
    track_lineage_ = con["track-lineage"];
    entity_name_ = std::string(con["entity"][0]);
    entity_config_ = con["entity"][1];

	initialize();
  }

  life::configuration publish_configuration() {
    life::configuration con;
    con["track-lineage"] = track_lineage_;
    con["size"] = size_;
    //con["DIR"] = dir_name_;
    con["load-from"] = load_spec_;
    con["entity"] = {entity_name_, {}};
    return con;
  }

  std::vector<life::entity> get_as_vector();
  void merge(std::vector<life::entity>);
  void snapshot(long)const;
  life::configuration get_stats(long) const;
  void prune_lineage(long);
  void flush_unpruned();
};
