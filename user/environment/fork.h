
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

class fork {

  life::environment_spec one_{"null_environment"};
  life::environment_spec two_{"null_environment"};
  life::environment one = life::make_environment(one_);
  life::environment two = life::make_environment(two_);

public:
  fork() { configure(publish_configuration()); }

  life::environment_spec publish_configuration()
  {
    life::environment_spec es{"fork"};

	es.bind_environment("one",one_);

	es.bind_environment("two",two_);

	es.bind_tag_inequality({"one", "post"}, {"two", "post"});

    return es;
  }

  void configure(life::environment_spec es)
  {
    es.configure_environment("one",one_);
    es.configure_environment("two",two_);
	one = life::make_environment(one_);
	two = life::make_environment(two_);
  }

  life::population evaluate(life::population);
};
