
#pragma once

#include "../../components.h"

#include <algorithm>
#include <iostream>
#include <vector>

class elite_replacement {

  double strength_ = 0.1;

public:
  void configure(life::configuration c) { strength_ = c["strength"]; }

  life::configuration publish_configuration() const {
    life::configuration c;
    c["strength"] = strength_;
    return c;
  }
  elite_replacement() { configure(publish_configuration()); }

  life::population evaluate(life::population);
};
