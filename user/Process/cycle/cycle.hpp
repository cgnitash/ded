
#pragma once

#include "../../../components.hpp"

class cycle
{

  long                        count_ = 1;
  ded::specs::ProcessSpec world_{ "null_process" };
  ded::concepts::Process  world = ded::makeProcess(world_);

public:
  cycle()
  {
    configure(publishConfiguration());
  }

  ded::specs::ProcessSpec
      publishConfiguration()
  {
    ded::specs::ProcessSpec es;

    es.bindParameter("count", count_);

    es.bindProcess("world", world_);

    return es;
  }

  void
      configure(ded::specs::ProcessSpec es)
  {
    es.configureParameter("count", count_);

    es.configureProcess("world", world_);

    world = ded::makeProcess(world_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
