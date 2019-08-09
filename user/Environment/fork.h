
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

    es.bindPostTag("info", "double");

	es.bindEnvironment("one",one_);
    es.bindEnvironmentPostConstraints("one", { { "info", "double" } });

	es.bindEnvironment("two",two_);
    es.bindEnvironmentPostConstraints("two", { { "info", "double" } });

    return es;
  }

  void configure(ded::specs::EnvironmentSpec es)
  {
    es.configureEnvironment("one",one_);
	one = ded::make_Environment(one_);

    es.configureEnvironment("two",two_);
	two = ded::make_Environment(two_);

    es.configurePostTag("info", info_tag_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
