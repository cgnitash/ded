
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

	es.bind_environment("first",first_);

	es.bind_environment("second",second_);

	es.bind_tag_equality({"first", "post"}, {"second", "pre"});

    return es;
  }

  void configure(ded::specs::EnvironmentSpec es)
  {
    es.configure_environment("first",first_);
    es.configure_environment("second",second_);
	first = ded::make_Environment(first_);
	second = ded::make_Environment(second_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
