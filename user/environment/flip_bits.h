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
  size_t  size_ = 10;
  double eval(life::entity &);

public:
  flip_bits() { configure(publish_configuration()); }

  life::configuration publish_configuration() {
    life::configuration c;
    c["parameters"]["size"] = size_;

    //  o:in:P must handle these tags
    c["pre-tags"] = nullptr;

	// o:in:P' must handle these tags 
    c["post-tags"]["flippiness"] = flippiness_tag_;

    return c;
  }

  void configure(life::configuration c) {
    size_ = c["parameters"]["size"];

    flippiness_tag_ = c["post-tags"]["flippiness"];
  }

// requires org.input("some-bits","vector<bool,size>")
// requires org.output("flipped-bits","vector<bool,size>")
  life::population evaluate(life::population);
// guarantess org.data["flippiness"] exists and is double
};

