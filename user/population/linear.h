#pragma once

#include "../../components.h"

#include <algorithm>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

class linear {

  bool                      track_lineage_{ false };
  long                      size_{ 0 };
  std::string               load_spec_;

  // count of descendants per entity
  std::vector<std::pair<life::entity, long>> fossils_;

  std::vector<life::entity> pop_;

  life::entity_spec org_{ "null_entity" };

  void update_tree(long p, int count);
  bool found_in_fossils(long) const;
  bool found_in_pop(long) const;
  void initialize();

public:
  linear() { configure(publish_configuration()); }

  void configure(life::population_spec ps)
  {
    ps.configure_parameter("size",size_);
    ps.configure_parameter("load_from",load_spec_);
    ps.configure_parameter("track_lineage",track_lineage_);

    ps.configure_entity(org_);

    initialize();
  }

  life::population_spec publish_configuration()
  {
    life::population_spec ps{"linear"};
    ps.bind_parameter("track_lineage",track_lineage_);
    ps.bind_parameter("size",size_);
    ps.bind_parameter("load_from",load_spec_);

    //con["parameters"]["entity"] = { entity_[0], {} };
	ps.bind_entity(org_);
    return ps;
  }

  size_t                    size() const { return pop_.size(); }
  std::vector<life::entity> get_as_vector()const ;
  void                      merge(std::vector<life::entity>);
  void                      snapshot(long) const;
  void                      prune_lineage(long);
  void                      flush_unpruned();
};
