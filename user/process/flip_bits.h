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

class flip_bits {

  std::string flippiness_tag_ = "flippiness,double";

  std::string org_input_tag_  = "in-bits,A<double,size>";
  std::string org_output_tag_ = "out-bits,A<double,size>";

  size_t size_ = 10;
  double eval(ded::substrate &);

public:
  flip_bits() { configure(publishConfiguration()); }

  ded::configuration publishConfiguration()
  {
    ded::configuration c;
    c["parameters"]["size"] = size_;

    c["pre-tags"] = nullptr;

    c["post-tags"]["flippiness"] = flippiness_tag_;

    c["input-tags"]["in-bits"] = org_input_tag_;
    c["output-tags"]["out-bits"]       = org_output_tag_;

    return c;
  }

  void configure(ded::configuration c)
  {
    size_ = c["parameters"]["size"];

    flippiness_tag_ = c["post-tags"]["flippiness"];

    org_input_tag_  = c["input-tags"]["in-bits"];
    org_output_tag_ = c["output-tags"]["out-bits"];
  }

  ded::population evaluate(ded::population);
};
