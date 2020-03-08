#pragma once

#include "../../../components.hpp"

#include <algorithm>
#include <vector>

class null_substrate
{

public:
  void
      configuration(ded::specs::SubstrateSpec &)
  {
  }

  void
      reset()
  {
  }
  void
      mutate()
  {
  }
  void input(std::string, ded::concepts::Signal)
  {
  }
  ded::concepts::Signal output(std::string)
  {
    return {};
  }
  void
      tick()
  {
  }
};
