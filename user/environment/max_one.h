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

class max_one {

	std::string ones_tag_ = "ones,double";
public:
  max_one() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() { 
	life::configuration c;  
    c["parameters"] = nullptr;

    //  o:in:P must handle these tags
    c["pre-tags"] = nullptr;

	// o:in:P' must handle these tags 
    c["post-tags"]["ones"] = ones_tag_;

    return c;
	  return life::configuration(); }

  void configure(life::configuration c) { ones_tag_ = c["post-tags"]["ones"]; }

  life::population evaluate(life::population);
};

