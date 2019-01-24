#pragma once

#include "../../components.h"

#include <algorithm>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

class linear {

  // count of descendants per entity
  std::vector<std::pair<life::entity, long>> fossils_;

  bool                      track_lineage_{ false };
  std::vector<life::entity> pop_;
  std::string               load_spec_{ "" };
  long                      size_{ 0 };

  life::configuration entity_{ "null_entity", {} };

  void update_tree(long p, int count);
  bool found_in_fossils(long) const;
  bool found_in_pop(long) const;
  void initialize();

public:
  linear() { configure(publish_configuration()); }

  void configure(life::configuration con)
  {
    size_          = con["parameters"]["size"];
    load_spec_     = std::string(con["parameters"]["load-from"]);
    track_lineage_ = con["parameters"]["track-lineage"];
    entity_        = con["parameters"]["entity"];

    initialize();
  }

  life::configuration publish_configuration()
  {
    life::configuration con;
    con["parameters"]["track-lineage"] = track_lineage_;
    con["parameters"]["size"]          = size_;
    con["parameters"]["load-from"]     = load_spec_;

    con["parameters"]["entity"] = { entity_[0], {} };
    return con;
  }

  size_t                    size() const { return pop_.size(); }
  std::vector<life::entity> get_as_vector();
  void                      merge(std::vector<life::entity>);
  void                      snapshot(long) const;
  void                      prune_lineage(long);
  void                      flush_unpruned();
};
