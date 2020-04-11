
#pragma once

#include "../../../components.hpp"

class cycle
{

public:
  void
      configuration(ded::specs::ProcessSpec &spec)
  {
    spec.parameter("count", count_);

    spec.nestedProcess("world", world_);

    world = ded::makeProcess(world_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);

private:
  long                    count_ = 1;
  ded::specs::ProcessSpec world_{ "null_process" };
  ded::concepts::Process  world = ded::makeProcess(world_);
};
