
#pragma once

#include "../../components.h"

#include <algorithm>

class fork {

  std::string info_tag_;// = "double";
  ded::specs::EnvironmentSpec one_{"null_environment"};
  ded::specs::EnvironmentSpec two_{"null_environment"};
  ded::concepts::Environment one = ded::make_Environment(one_);
  ded::concepts::Environment two = ded::make_Environment(two_);

public:
  fork() { configure(publish_configuration()); }

  ded::specs::EnvironmentSpec publish_configuration()
  {
    ded::specs::EnvironmentSpec es;

    es.bind_post("info", "double");

	es.bind_environment("one",one_);
    es.bind_environment_post_constraints("one", { { "info", "double" } });

	es.bind_environment("two",two_);
    es.bind_environment_post_constraints("two", { { "info", "double" } });

    return es;
  }

  void configure(ded::specs::EnvironmentSpec es)
  {
    es.configure_environment("one",one_);
	one = ded::make_Environment(one_);

    es.configure_environment("two",two_);
	two = ded::make_Environment(two_);

    es.configure_post("info", info_tag_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
