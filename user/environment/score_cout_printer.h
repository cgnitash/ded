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

class score_cout_printer {

	bool message_ = true;

	std::string used_score_tag_ = "score";

	std::string env_name_ = "null_environment";
	life::configuration env_config_;
	life::configuration env_reqs_;
public:
  score_cout_printer() { configure(publish_configuration()); }

  life::configuration publish_configuration() {
    life::configuration c;
    c["parameters"]["message"] = message_;

    //  o:in:P must handle these tags
    c["pre-tags"] = used_score_tag_;

	// o:in:P' has no tags
    c["post-tags"] = nullptr;

    //  o:in:env(P) must provide these tags
    c["parameters"]["env"] = {
        env_name_,
        {},
        {},
        used_score_tag_}; // as well as propogate population requirements

    return c;
  }

  void configure(life::configuration con) {
    message_ = con["parameters"]["message"];
    env_name_ = con["parameters"]["env"][0];
    env_config_ = con["parameters"]["env"][1];

  }

// guarantess org.data["x"] exists and is double
  life::population evaluate(life::population);
// guarantess org.data["fx"] exists and is double
};

