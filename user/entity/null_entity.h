# pragma once

#include"../../core/configuration.h"

#include <vector>
#include <algorithm>


class null_entity {

public:
  void configure(life::configuration) {}
  null_entity() { configure(publish_configuration()) ;}
  life::configuration publish_configuration() {
    life::configuration c;
    return c;
  }
  void mutate() ;
  long update() const ;

};
