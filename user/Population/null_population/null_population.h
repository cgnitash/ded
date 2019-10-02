#pragma once

#include "../../../components.h"

#include <algorithm>
#include <vector>

class null_population {

public:
  null_population() { configure(publishConfiguration()); }
  void configure(ded::specs::PopulationSpec) {}
  ded::specs::PopulationSpec publishConfiguration()
  {
    //ded::specs::PopulationSpec ps{"null_population"};
    return {};
  }
  std::vector<ded::concepts::Substrate> getAsVector() const { return {}; }

  size_t size() const { return {}; }
  void   merge(std::vector<ded::concepts::Substrate>) {}
  void   clear() {}
  void   pruneLineage(long) {}
  void   snapShot(long) {}
  void   flushUnpruned() {}
};
