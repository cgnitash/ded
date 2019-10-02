#pragma once

#include "../../../components.h"

#include <algorithm>
#include <vector>

class null_substrate
{

public:
  void configure(ded::specs::SubstrateSpec)
  {
  }
  null_substrate()
  {
    configure(publishConfiguration());
  }
  ded::specs::SubstrateSpec
      publishConfiguration()
  {
    return {};
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
