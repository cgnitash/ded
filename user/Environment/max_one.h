#pragma once

#include "../../components.h"

#include <algorithm>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <vector>

class max_one {

  long        length_              = 10;
  std::string ones_tag_            = "double";
  std::string org_output_ones_tag_ = "<double,length>";

public:
  max_one() { configure(publish_configuration()); }

  ded::specs::EnvironmentSpec publish_configuration()
  {
    ded::specs::EnvironmentSpec es;
    es.bind_parameter("length",length_);
 
    es.bind_post("ones","double");

    es.bind_output("ons","<double,length>");
    return es;
  }
  void configure(ded::specs::EnvironmentSpec es)
  {

    es.configure_parameter("length",length_);

    es.configure_post("ones",ones_tag_);

    es.configure_output("ons",org_output_ones_tag_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
