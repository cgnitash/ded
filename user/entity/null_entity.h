# pragma once

#include"../../components.h"

#include <vector>
#include <algorithm>


class null_entity {

public:
  void configure(life::configuration) {}
  null_entity() { configure(publish_configuration()) ;}
  life::configuration publish_configuration() {
    life::configuration con;
    con["parameters"] = nullptr;
    return con;
  }

  void mutate();
  void input(life::signal);
  life::signal output();
  void tick();
};
