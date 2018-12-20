# pragma once

#include"../../components.h"

#include <vector>
#include <algorithm>


class bit_brain {

  size_t size_ = 8;
  life::encoding genome_ ; 

public:
  bit_brain() { configure(publish_configuration()); }

  void configure(life::configuration c) {
    size_ = c["size"];
	// encoding is simply a random start sequence
    genome_.generate(size_);
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
