
#pragma once

#include "../../components.h"

#include <string>

class smoosh {

  std::string d1_tag_ = "double";
  std::string d2_tag_ = "double";
  std::string wd_tag_ = "double";
  double      weight_  = 0.5;

public:
  smoosh() { configure(publish_configuration()); }

  ded::specs::EnvironmentSpec publish_configuration()
  {
    ded::specs::EnvironmentSpec es;
    es.bind_parameter("weight",weight_);

    es.bind_pre("d1", "double");
    es.bind_pre("d2", "double");

    es.bind_post("wd", "double");

    return es;
  }

  void configure(ded::specs::EnvironmentSpec es)
  {
    es.configure_parameter("weight",weight_);

    es.configure_pre("d1",d1_tag_);
    es.configure_pre("d2",d2_tag_);

    es.configure_post("wd",wd_tag_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
