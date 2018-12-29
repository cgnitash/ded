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
  life::configuration publish_configuration() {
    life::configuration con;
    con["parameters"] = nullptr;
    return con;
  }
  std::vector<life::entity> get_as_vector() {
	  return {};
  }
  void merge(std::vector<life::entity>) {}
  void clear() {}
  void snapshot(long) const {}
  life::configuration get_stats(long) const { return {}; }
  void prune_lineage(long) {}
  void flush_unpruned() {}
};
