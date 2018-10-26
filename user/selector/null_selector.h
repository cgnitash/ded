
#pragma once 

#include "../../core/configuration.h"
#include "../../core/entity.h"

#include <algorithm>
#include <iostream>
#include <vector>


class null_selector{

public:
 void  configure(life::configuration ) {}

life::configuration publish_configuration() const  {
  life::configuration c;
  return c;
}
  null_selector () {configure(publish_configuration()); }

  std::vector<life::entity> select(std::vector<life::entity> &);
};
