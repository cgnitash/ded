#pragma once

#include "../../core/configuration.h"
#include "../../core/encoding.h"
#include "../../core/entity.h"
#include "../../core/signal.h"

#include <algorithm>
#include <vector>

class null_population {

public:
  void configure(life::configuration) {}
  null_population() { configure(publish_configuration()); }
  life::configuration publish_configuration()
  {
    life::configuration con;
    con["parameters"] = nullptr;
    return con;
  }
  std::vector<life::entity> get_as_vector() { return {}; }

  size_t size() const { return {}; }
  void   merge(std::vector<life::entity>) {}
  void   clear() {}
  void   prune_lineage(long) {}
  void   snapshot(long) {}
  void   flush_unpruned() {}
};
