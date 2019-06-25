
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

class sequence {

  life::environment_spec first_{"null_environment"};
  life::environment_spec second_{"null_environment"};
  life::environment first = life::make_environment(first_);
  life::environment second = life::make_environment(second_);

public:
  sequence() { configure(publish_configuration()); }

  life::environment_spec publish_configuration()
  {
    life::environment_spec es;//{"sequence"};

	es.bind_environment("first",first_);

	es.bind_environment("second",second_);

	es.bind_tag_equality({"first", "post"}, {"second", "pre"});

    return es;
  }

  void configure(life::environment_spec es)
  {
    es.configure_environment("first",first_);
    es.configure_environment("second",second_);
	first = life::make_environment(first_);
	second = life::make_environment(second_);
  }

  life::population evaluate(life::population);
};
