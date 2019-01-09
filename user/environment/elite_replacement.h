
#pragma once

#include "../../components.h"

#include <algorithm>
#include <iostream>
#include <vector>

class elite_replacement {

  std::string value_tag_ = "value,double";
  double strength_ = 0.1;

public:
  void configure(life::configuration c) { 
	  strength_ = c["parameters"]["strength"]; 
   
	  value_tag_ = c["pre-tags"]["value"];
  }

  life::configuration publish_configuration() {
    life::configuration c;
    c["parameters"]["strength"] = strength_;

    //  o:in:P must handle these tags
    c["pre-tags"]["value"] = value_tag_;

    c["org-inputs"] = nullptr;
    c["org-outputs"] = nullptr;

	// o:in:P' has no tags
	c["post-tags"] = nullptr;

    return c;
  }
  elite_replacement() { configure(publish_configuration()); }

  life::population evaluate(life::population);
};
