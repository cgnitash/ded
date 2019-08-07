
#pragma once

#include "../../components.h"

#include <string>

class elite_replacement {

  std::string value_tag_ = "double";
  double      strength_  = 0.1;
  int invoke_ = 0;

public:
  elite_replacement() { configure(publish_configuration()); }

  ded::specs::EnvironmentSpec publish_configuration()
  {
    ded::specs::EnvironmentSpec es;

    es.bind_parameter("strength",
                      strength_,
                      { { [](double s) { return 0.0 < s && s < 1.0; },
                          "strength must be in the range (0.0, 1.0)" } });

    es.bind_pre("value", "double");

    return es;
  }

  void configure(ded::specs::EnvironmentSpec es)
  {
    es.configure_parameter("strength",strength_);

    es.configure_pre("value",value_tag_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};

