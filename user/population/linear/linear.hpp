#pragma once

#include "../../../components.hpp"

#include <algorithm>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

class linear
{

public:
  void
      configuration(ded::specs::PopulationSpec &ps)
  {
    ps.parameter("size", size_);
    ps.parameter("load_from", load_spec_);
    ps.parameter("track_lineage", track_lineage_);

    ps.nestedSubstrate(org_);

    initialize();
  }

  size_t
      size() const
  {
    return pop_.size();
  }
  std::vector<ded::concepts::Substrate> getAsVector() const;
  void merge(std::vector<ded::concepts::Substrate>);
  void snapShot(long) const;
  void pruneLineage(long);
  void flushUnpruned();

private:
  bool        track_lineage_{ false };
  long        size_{ 100 };
  std::string load_spec_;

  // count of descendants per substrate
  std::vector<std::pair<ded::concepts::Substrate, long>> fossils_;

  std::vector<ded::concepts::Substrate> pop_;

  ded::specs::SubstrateSpec org_{ "null_substrate" };

  void update_tree(long p, int count);
  bool found_in_fossils(long) const;
  bool found_in_pop(long) const;
  void initialize();
};
