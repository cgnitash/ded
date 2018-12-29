#pragma once

#include"../../components.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>
#include <random>
#include <regex>
#include <set>
#include <initializer_list>

class replicate {

	size_t num_ = 1;
	std::string env_name_ = "null_environment";
	life::configuration env_config_;
	life::configuration env_reqs_;
public:
  replicate() { configure(publish_configuration()); }

  life::configuration publish_configuration() {
    life::configuration c;
	c["parameters"]["num"] = num_;
	c["parameters"]["env"] = {env_name_,{}}; // as well as propogate population requirements
    return c;
  }

  void configure(life::configuration con) {
    num_ = con["parameters"]["num"];
    env_name_ = con["parameters"]["env"][0];
	env_config_ = con["parameters"]["env"][1];
  }

// guarantess org.data["x"] exists and is double
  life::population evaluate(life::population);
// guarantess org.data["fx"] exists and is double
};

