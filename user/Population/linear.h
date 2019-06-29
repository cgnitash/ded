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
  std::vector<std::pair<ded::concepts::Entity, long>> fossils_;

  std::vector<ded::concepts::Entity> pop_;

  ded::specs::EntitySpec org_{ "null_entity" };

  void update_tree(long p, int count);
  bool found_in_fossils(long) const;
  bool found_in_pop(long) const;
  void initialize();

public:
  linear() { configure(publish_configuration()); }

  void configure(ded::specs::PopulationSpec ps)
  {
    ps.configure_parameter("size",size_);
    ps.configure_parameter("load_from",load_spec_);
    ps.configure_parameter("track_lineage",track_lineage_);

    ps.configure_entity(org_);

    initialize();
  }

  ded::specs::PopulationSpec publish_configuration()
  {
    ded::specs::PopulationSpec ps;//{"linear"};
    ps.bind_parameter("track_lineage",track_lineage_);
    ps.bind_parameter("size",size_);
    ps.bind_parameter("load_from",load_spec_);

    //con["parameters"]["entity"] = { entity_[0], {} };
	ps.bind_entity(org_);
    return ps;
  }

  size_t                    size() const { return pop_.size(); }
  std::vector<ded::concepts::Entity> get_as_vector()const ;
  void                      merge(std::vector<ded::concepts::Entity>);
  void                      snapshot(long) const;
  void                      prune_lineage(long);
  void                      flush_unpruned();
};
