
#pragma once

#include "../../components.h"

class sequence {

  ded::specs::EnvironmentSpec first_{"null_environment"};
  ded::specs::EnvironmentSpec second_{"null_environment"};
  ded::concepts::Environment first = ded::make_Environment(first_);
  ded::concepts::Environment second = ded::make_Environment(second_);

public:
  sequence() { configure(publish_configuration()); }

  ded::specs::EnvironmentSpec publish_configuration()
  {
    ded::specs::EnvironmentSpec es;

	es.bindEnvironment("first",first_);

	es.bindEnvironment("second",second_);

	es.bindTagEquality({"first", "post"}, {"second", "pre"});

    return es;
  }

  void configure(ded::specs::EnvironmentSpec es)
  {
    es.configureEnvironment("first",first_);
    es.configureEnvironment("second",second_);
	first = ded::make_Environment(first_);
	second = ded::make_Environment(second_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
