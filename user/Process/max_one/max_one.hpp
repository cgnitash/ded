#pragma once

#include "../../../components.hpp"

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
  max_one() { configure(publishConfiguration()); }

  ded::specs::ProcessSpec publishConfiguration()
  {
    ded::specs::ProcessSpec es;
    es.bindParameter("length",length_);
 
    es.bindPostTag("ones","double");

    es.bindOutput("ons","<double,length>");
    return es;
  }
  void configure(ded::specs::ProcessSpec es)
  {

    es.configureParameter("length",length_);

    es.configurePostTag("ones",ones_tag_);

    es.configureOutput("ons",org_output_ones_tag_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
