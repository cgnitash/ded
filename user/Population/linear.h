#pragma once

#include "../../components.h"

#include <algorithm>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

class linear {

  bool                      track_lineage_{ false };
  long                      size_{ 100 };
  std::string               load_spec_;

  // count of descendants per substrate
  std::vector<std::pair<ded::concepts::Substrate, long>> fossils_;

  std::vector<ded::concepts::Substrate> pop_;

  ded::specs::SubstrateSpec org_{ "null_substrate" };

  void update_tree(long p, int count);
  bool found_in_fossils(long) const;
  bool found_in_pop(long) const;
  void initialize();

public:
  linear() { configure(publishConfiguration()); }

  void configure(ded::specs::PopulationSpec ps)
  {
    ps.configureParameter("size",size_);
    ps.configureParameter("load_from",load_spec_);
    ps.configureParameter("track_lineage",track_lineage_);

    ps.configureSubstrate(org_);

    initialize();
  }

  ded::specs::PopulationSpec publishConfiguration()
  {
    ded::specs::PopulationSpec ps;//{"linear"};
    ps.bindParameter("track_lineage",track_lineage_);
    ps.bindParameter(
        "size",
        size_,
        { { [](long s) { return s > 0; },
            "linear population must contain at least 1 substrate" } });
    ps.bindParameter("load_from",load_spec_);

    //con["parameters"]["substrate"] = { substrate_[0], {} };
	ps.bindSubstrate(org_);
    return ps;
  }

  size_t                    size() const { return pop_.size(); }
  std::vector<ded::concepts::Substrate> getAsVector()const ;
  void                      merge(std::vector<ded::concepts::Substrate>);
  void                      snapShot(long) const;
  void                      pruneLineage(long);
  void                      flushUnpruned();
};
