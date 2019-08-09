
#pragma once

#include "../../components.h"

class cycle
{

  long                        count_ = 1;
  ded::specs::EnvironmentSpec world_{ "null_environment" };
  ded::concepts::Environment  world = ded::make_Environment(world_);

public:
  cycle()
  {
    configure(publish_configuration());
  }

  ded::specs::EnvironmentSpec
      publish_configuration()
  {
    ded::specs::EnvironmentSpec es;

    es.bindParameter("count", count_);

    es.bindEnvironment("world", world_);

    es.bindTagEquality({ "world", "pre" }, { "world", "post" });

    return es;
  }

  void
      configure(ded::specs::EnvironmentSpec es)
  {
    es.configureParameter("count", count_);

    es.configureEnvironment("world", world_);

    world = ded::make_Environment(world_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
