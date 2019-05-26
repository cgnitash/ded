#pragma once

#include "../../core/configuration.h"
#include "../../core/encoding.h"
#include "../../core/entity.h"
#include "../../core/signal.h"

#include <algorithm>
#include <vector>

class null_population {

public:
  null_population() { configure(publish_configuration()); }
  void configure(life::population_spec) {}
  life::population_spec publish_configuration()
  {
    life::population_spec ps{"null_population"};
    //con["parameters"] = nullptr;
    return ps;
  }
  std::vector<life::entity> get_as_vector() { return {}; }

  size_t size() const { return {}; }
  void   merge(std::vector<life::entity>) {}
  void   clear() {}
  void   prune_lineage(long) {}
  void   snapshot(long) {}
  void   flush_unpruned() {}
};
