
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

	long num_ = 1;
	std::string env_name_ = "null_environment";
	life::configuration env_config_;
public:
  replicate() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() {
    life::configuration c;
	c["num"] = num_;
	c["env"] = {env_name_,{}};
    return c;
  }

  void configure(life::configuration c) {
    num_ = c["num"];
    env_name_ = c["env"][0];
	env_config_ = c["env"][1];
  }

  void evaluate(std::vector<life::entity> &);
};

