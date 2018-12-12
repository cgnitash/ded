# pragma once

#include"../../core/configuration.h"
#include"../../core/signal.h"
#include"../../core/encoding.h"
#include"../../core/entity.h"

#include <vector>
#include <algorithm>


class null_population {

public:
  void configure(life::configuration) {}
  null_population() { configure(publish_configuration()) ;}
  life::configuration publish_configuration() { return life::configuration(); }

  std::vector<life::entity> get_as_vector() {
	  return {};
  }
  void merge(std::vector<life::entity>) {}
  void clear() {}
  void snapshot(std::ostream &, long) const {}
  void get_stats(std::ostream &, long) const {}
  void prune_lineage(std::ostream &, std::ostream &, long) {}
};
