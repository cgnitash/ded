#pragma once

#include "../../components.h"

#include <algorithm>
#include <vector>

class null_entity
{

public:
  void configure(ded::specs::EntitySpec)
  {
  }
  null_entity()
  {
    configure(publish_configuration());
  }
  ded::specs::EntitySpec
      publish_configuration()
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
