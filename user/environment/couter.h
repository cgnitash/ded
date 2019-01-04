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

class couter {

	bool message_ = true;

	std::string score_tag_ = "score";

public:
  couter() { configure(publish_configuration()); }

  life::configuration publish_configuration() {
    life::configuration c;
    c["parameters"]["message"] = message_;

    //  o:in:P must handle these tags
    c["pre-tags"] = nullptr;

	// o:in:P' has no tags
    c["post-tags"] = nullptr;

    return c;
  }

  void configure(life::configuration con) {
    message_ = con["parameters"]["message"];

  }

  life::population evaluate(life::population);
};

