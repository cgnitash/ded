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
  void snapshot(std::ofstream&)const {}
  life::configuration get_stats(){ return {}; }
  std::vector<life::entity> prune_lineage() { return {}; }
};
