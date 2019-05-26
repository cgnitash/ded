
#pragma once

#include "../../components.h"

#include <algorithm>
#include <iostream>
#include <vector>

class elite_replacement {

  std::string value_tag_ = "value,double";
  double      strength_  = 0.1;

public:
  void configure(life::environment_spec es)
  {
    es.configure_parameter("strength",strength_);

    es.configure_pre("value",value_tag_);
  }

  life::environment_spec publish_configuration()
  {
    life::environment_spec es{"elite_replacement"};
    es.bind_parameter("strength",strength_);

    //  o:in:P must handle these tags
    es.bind_pre("value",value_tag_);

    //c["input-tags"]  = nullptr;
    //c["output-tags"] = nullptr;

    // o:in:P' has no tags
    //ic["post-tags"] = nullptr;

    return es;
  }
  elite_replacement() { configure(publish_configuration()); }

  life::population evaluate(life::population);
};
