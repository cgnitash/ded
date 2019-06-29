
#pragma once

#include "../../components.h"

class cycle {

  long count_     = 1;

  ded::specs::EnvironmentSpec world_{"null_environment"};
  ded::concepts::Environment world = ded::make_Environment(world_);

public:
  cycle() { configure(publish_configuration()); }

  ded::specs::EnvironmentSpec publish_configuration()
  {
    ded::specs::EnvironmentSpec es;

    es.bind_parameter("count",count_);

	es.bind_environment("world",world_);

	es.bind_tag_equality({"world","pre"},{"world","post"});

    return es;
  }

  void configure(ded::specs::EnvironmentSpec es)
  {
    es.configure_parameter("count",count_);

    es.configure_environment("world",world_);

    world = ded::make_Environment(world_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
