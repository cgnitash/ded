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

class replicate {

  long num_ = 1;

  std::string fx_tag_     = "fx,double";
  std::string used_x_tag_ = "x,double";

  //life::configuration env_{ "null_environment", {} };
  life::environment_spec env_{ ""};

public:
  replicate() { configure(publish_configuration()); }

  life::environment_spec publish_configuration()
  {
    life::environment_spec es{"replicate"};
    es.bind_parameter("num",num_);

    // o:in:P has no tags
    //c["pre-tags"] = nullptr;

    //  o:in:P' must handle these tags
    es.bind_post("fx",fx_tag_);

    //c["input-tags"]  = nullptr;
    //c["output-tags"] = nullptr;

    //  o:in:env(P) must provide these tags
    //c["parameters"]["env"] = {
    //  env_[0], {}, {}, { used_x_tag_ }
    //};   // as well as propogate population requirements
	es.bind_environment("env",env_);
	es.bind_environment_post_constraints("env", { used_x_tag_ });
    return es;
  }

  void configure(life::environment_spec es)
  {
    es.configure_parameter("num",num_);

    es.configure_environment("env",env_);

    es.configure_post("fx",fx_tag_);

  }

  life::population evaluate(life::population);
};
