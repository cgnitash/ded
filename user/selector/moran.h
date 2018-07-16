
#pragma once 

#include "../../core/configuration.h"
#include "../../core/entity.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <random>


class moran {
	
	bool ran_;

	public:
  moran() : moran(publish_configuration()) {}
  moran(life::configuration c) : ran_(std::stol(c["random"].first)) {}

  life::configuration publish_configuration() const  {
  life::configuration c;
  c["random"] = {"0", "random replacement"};
  return c;
}

std::vector<life::entity> select(std::vector<life::entity> &pop) const ;
};
