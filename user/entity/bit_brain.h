# pragma once

#include"../../core/configuration.h"
#include"../../core/signal.h"
#include"../../core/encoding.h"

#include <vector>
#include <algorithm>


class bit_brain {

  size_t size_ = 8;
  life::encoding genome_ = life::encoding(size_, 0);

public:
  bit_brain() { configure(publish_configuration()); }

  void configure(life::configuration c) {
    size_ = c["size"];
	// encoding is simply the modulo 2 values 
    genome_ = life::encoding(size_, 0);
  }

  life::configuration publish_configuration() {
    life::configuration c;
    c["size"] = size_;
    return c;
  }
  void mutate();
  void input(life::signal);
  life::signal output();
  void tick();

};
