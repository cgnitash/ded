
#pragma once

#include "../../components.h"

#include <string>

class smoosh {

  std::string d1_tag_ = "double";
  std::string d2_tag_ = "double";
  std::string wd_tag_ = "double";
  double      weight_  = 0.5;

public:
  smoosh() { configure(publishConfiguration()); }

  ded::specs::ProcessSpec publishConfiguration()
  {
    ded::specs::ProcessSpec es;
    es.bindParameter("weight",weight_);

    es.bindPreTag("d1", "double");
    es.bindPreTag("d2", "double");

    es.bindPostTag("wd", "double");

    return es;
  }

  void configure(ded::specs::ProcessSpec es)
  {
    es.configureParameter("weight",weight_);

    es.configurePreTag("d1",d1_tag_);
    es.configurePreTag("d2",d2_tag_);

    es.configurePostTag("wd",wd_tag_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
