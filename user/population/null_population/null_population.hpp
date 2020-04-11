#pragma once

#include "../../../components.hpp"

#include <algorithm>
#include <vector>

class null_population
{

public:
  void
      configuration(ded::specs::PopulationSpec &)
  {
  }

  std::vector<ded::concepts::Substrate>
      getAsVector() const
  {
    return {};
  }

  size_t
      size() const
  {
    return {};
  }
  void merge(std::vector<ded::concepts::Substrate>)
  {
  }
  void
      clear()
  {
  }
  void
      pruneLineage(long)
  {
  }
  void
      snapShot(long)
  {
  }
  void
      flushUnpruned()
  {
  }
};
