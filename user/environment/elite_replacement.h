
#pragma once

#include "../../components.h"

#include <algorithm>
#include <iostream>
#include <vector>

class elite_replacement {

  std::string value_tag_ = "double";
  double      strength_  = 0.1;
  int invoke_ = 0;

public:
  elite_replacement() { configure(publish_configuration()); }

  life::environment_spec publish_configuration()
  {
    life::environment_spec es;//{"elite_replacement"};
    es.bind_parameter("strength",strength_);

    es.bind_pre("value", "double");

    return es;
  }

  void configure(life::environment_spec es)
  {
    es.configure_parameter("strength",strength_);

    es.configure_pre("value",value_tag_);
  }

  life::population evaluate(life::population);
};
