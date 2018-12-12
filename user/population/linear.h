#pragma once

#include "../../core/configuration.h"
#include "../../core/encoding.h"
#include "../../core/entity.h"
#include "../../core/signal.h"
#include "../../core/utilities.h"

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
  life::configuration entity_config_;
  long size_{0};

  void update_tree(long p, int count);

public:
  linear() { configure(publish_configuration()); }

  void configure(life::configuration con) {
    size_ = con["size"];
    track_lineage_ = con["track-lineage"];
    entity_name_ = std::string(con["entity"][0]);
    entity_config_ = con["entity"][1];

    pop_.clear();
    ranges::generate_n(ranges::back_inserter(pop_), size_, [&] {
      auto org = life::make_entity(entity_name_);
      org.configure(entity_config_);
      if (track_lineage_) 
		fossils_.push_back({org,1});
      return org;
    });
  }

  life::configuration publish_configuration() {
    life::configuration con;
    con["track-lineage"] = track_lineage_;
    con["size"] = size_;
    con["entity"] = {entity_name_, {}};
    return con;
  }

  std::vector<life::entity> get_as_vector();
  void merge(std::vector<life::entity>);
  void snapshot(std::ostream&,long)const;
  void get_stats(std::ostream &, long) const;
  void prune_lineage(std::ostream &, std::ostream &, long);
};
