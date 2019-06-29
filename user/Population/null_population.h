#pragma once

#include "../../core/configuration.h"
#include "../../core/concepts/encoding.h"
#include "../../core/concepts/entity.h"
#include "../../core/concepts/signal.h"

#include <algorithm>
#include <vector>

class null_population {

public:
  null_population() { configure(publish_configuration()); }
  void configure(ded::specs::PopulationSpec) {}
  ded::specs::PopulationSpec publish_configuration()
  {
    //ded::specs::PopulationSpec ps{"null_population"};
    return {};
  }
  std::vector<ded::concepts::Entity> get_as_vector() const { return {}; }

  size_t size() const { return {}; }
  void   merge(std::vector<ded::concepts::Entity>) {}
  void   clear() {}
  void   prune_lineage(long) {}
  void   snapshot(long) {}
  void   flush_unpruned() {}
};
