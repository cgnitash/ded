#pragma once

#include "../../components.h"

#include <algorithm>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class edlund_maze {

  std::string distance_tag_ = "distance,double";

  std::string org_input_vision_tag_  = "vision,A<double,2>";
  std::string org_output_action_tag_ = "action,A<double,2>";

  size_t           doors_ = 10;
  size_t           corridor_length_ = 10;
  size_t           updates_ = 100;
  std::vector<size_t> door_positions_;

  double eval(life::entity &);

public:
  edlund_maze() { configure(publish_configuration()); }

  life::configuration publish_configuration()
  {
    life::configuration ec;
    ec["parameters"]["doors"] = doors_;
    ec["parameters"]["updates"]=updates_  ;
    ec["parameters"]["corridor-length"]=corridor_length_  ;

    ec["pre-tags"] = nullptr;

    ec["post-tags"]["distance"] = distance_tag_;

    ec["input-tags"]["vision"]  = org_input_vision_tag_;
    ec["output-tags"]["action"] = org_output_action_tag_;
    return ec;
  }

  void configure(life::configuration con)
  {

    doors_ = con["parameters"]["doors"];
	door_positions_.reserve(doors_);
    updates_ = con["parameters"]["updates"];
    corridor_length_ = con["parameters"]["corridor-length"];

    distance_tag_ = con["post-tags"]["distance"];

    org_input_vision_tag_  = con["input-tags"]["vision"];
    org_output_action_tag_ = con["output-tags"]["action"];
  }

  life::population evaluate(life::population);
};
