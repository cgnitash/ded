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

class flip_bits {

  std::string flippiness_tag_ = "flippiness,double";

  std::string org_input_tag_ = "in-bits,A<double,size>";
  std::string org_output_tag_ = "out-bits,A<double,size>";

  size_t  size_ = 10;
  double eval(life::entity &);

public:
  flip_bits() { configure(publish_configuration()); }

  life::configuration publish_configuration() {
    life::configuration c;
    c["parameters"]["size"] = size_;

    c["pre-tags"] = nullptr;

    c["post-tags"]["flippiness"] = flippiness_tag_;

	c["input-tags"]["line-of-sight"] = org_input_tag_; 
	c["output-tags"]["action"] = org_output_tag_; 

    return c;
  }

  void configure(life::configuration c) {
    size_ = c["parameters"]["size"];

    flippiness_tag_ = c["post-tags"]["flippiness"];

    org_input_tag_ = c["input-tags"]["line-of-sight"];
    org_output_tag_ = c["output-tags"]["action"];
  }

  life::population evaluate(life::population);
};

