# pragma once

#include"../../core/configuration.h"
#include"../../core/signal.h"
#include"../../core/encoding.h"

#include <vector>
#include <algorithm>


class bit_brain {

  long size_;
	life::encoding genome_;

public:
  void configure(life::configuration c) {
    size_ = c["size"];
    genome_ = life::encoding(size_, 0);
  }
  bit_brain() { configure(publish_configuration()) ;}
  life::configuration publish_configuration() {
    life::configuration c;
    c["size"] = 8;
    return c;
  }
  void mutate();
  void input(life::signal);
  life::signal output();
  void tick();

};
