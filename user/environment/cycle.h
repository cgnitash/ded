
#pragma once

#include "../../components.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

class cycle {

  long count_     = 1;

  life::environment_spec world_{"null_environment"};
  life::environment world = life::make_environment(world_);

public:
  cycle() { configure(publish_configuration()); }

  life::environment_spec publish_configuration()
  {
    life::environment_spec es;//{"cycle"};

    es.bind_parameter("count",count_);

	es.bind_environment("world",world_);

	es.bind_tag_equality({"world","pre"},{"world","post"});

    return es;
  }

  void configure(life::environment_spec es)
  {
    es.configure_parameter("count",count_);

    es.configure_environment("world",world_);

    world = life::make_environment(world_);
  }

  life::population evaluate(life::population);
};
