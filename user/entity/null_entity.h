# pragma once

#include"../../core/configuration.h"
#include"../../core/signal.h"
#include"../../core/encoding.h"

#include <vector>
#include <algorithm>


class null_entity {

public:
  void configure(life::configuration) {}
  null_entity() { configure(publish_configuration()) ;}
  life::configuration publish_configuration() { return life::configuration(); }

  void mutate();
  void input(life::signal);
  life::signal output();
  void tick();
};
