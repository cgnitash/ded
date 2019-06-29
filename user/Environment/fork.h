
#pragma once

#include "../../components.h"

#include <algorithm>
class fork {

  ded::specs::EnvironmentSpec one_{"null_environment"};
  ded::specs::EnvironmentSpec two_{"null_environment"};
  ded::concepts::Environment one = ded::make_Environment(one_);
  ded::concepts::Environment two = ded::make_Environment(two_);

public:
  fork() { configure(publish_configuration()); }

  ded::specs::EnvironmentSpec publish_configuration()
  {
    ded::specs::EnvironmentSpec es;

	es.bind_environment("one",one_);

	es.bind_environment("two",two_);

	es.bind_tag_inequality({"one", "post"}, {"two", "post"});

    return es;
  }

  void configure(ded::specs::EnvironmentSpec es)
  {
    es.configure_environment("one",one_);
    es.configure_environment("two",two_);
	one = ded::make_Environment(one_);
	two = ded::make_Environment(two_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
