
#pragma once 

#include "../../core/configuration.h"
#include "../../core/entity.h"

#include <algorithm>
#include <iostream>
#include <vector>


class elite_replacement {

		double strength_ = 0.5;
public:
  elite_replacement(life::configuration c) : strength_(std::stod(c["st"].first)) {}

life::configuration publish_configuration() const  {
  life::configuration c;
  c["st"] = {".2", "strength of replacement"};
  return c;
}
  elite_replacement() : elite_replacement(publish_configuration()) {}

  std::vector<life::entity> select(std::vector<life::entity> &pop) const ;
};
