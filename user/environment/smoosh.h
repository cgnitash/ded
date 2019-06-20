
#pragma once

#include "../../components.h"

#include <algorithm>
#include <iostream>
#include <vector>

class smoosh {

  std::string d1_tag_ = "d1,double";
  std::string d2_tag_ = "d2,double";
  std::string wd_tag_ = "wd,double";
  double      weight_  = 0.5;

public:
  smoosh() { configure(publish_configuration()); }

  life::environment_spec publish_configuration()
  {
    life::environment_spec es{"smoosh"};
    es.bind_parameter("weight",weight_);

    es.bind_pre("d1",d1_tag_);
    es.bind_pre("d2",d2_tag_);

    es.bind_post("wd",wd_tag_);

    return es;
  }

  void configure(life::environment_spec es)
  {
    es.configure_parameter("weight",weight_);

    es.configure_pre("d1",d1_tag_);
    es.configure_pre("d2",d2_tag_);

    es.configure_post("wd",wd_tag_);
  }

  life::population evaluate(life::population);
};
