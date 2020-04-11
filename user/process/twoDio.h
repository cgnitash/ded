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

class twoDio {

  int         size_{ 0 };
  std::string org_input_tag_  = "in,A<double,2>";
  std::string org_output_tag_ = "out,A<double,size>";

public:
  twoDio() { configure(publishConfiguration()); }

  ded::configuration publishConfiguration()
  {
    ded::configuration c;

    c["parameters"]["size"] = size_;

    c["pre-tags"] = nullptr;

    c["post-tags"] = nullptr;

    c["input-tags"]["in"]   = org_input_tag_;

    c["output-tags"]["out"] = org_output_tag_;

    return c;
  }

  void configure(ded::configuration con) {
    size_= con["parameters"]["size"] ;
    org_input_tag_  = con["input-tags"]["in"];
    org_output_tag_ = con["output-tags"]["out"];
  }

  ded::population evaluate(ded::population);
};
