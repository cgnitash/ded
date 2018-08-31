
#pragma once 

#include "../../core/configuration.h"
#include "../../core/entity.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <random>

// will be used as a 'selector'
class moran {

  bool ran_;

public:
  moran() { configure(publish_configuration()); }

  life::configuration publish_configuration() const {
    life::configuration c;
    c["random"] = false;
    return c;
  }
  void configure(life::configuration c) { ran_ = c["random"]; }

  std::vector<life::entity> select(std::vector<life::entity> &pop) const;
};
