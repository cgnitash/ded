# pragma once

#include"../../core/configuration.h"

#include <vector>
#include <algorithm>


class bit_brain {

  long size_;
  std::vector<bool> encoding_;

public:
  bit_brain(life::configuration c)
      : size_(std::stol(c["size"].first)),
        encoding_(std::vector<bool>(size_, false)) {}

  bit_brain() : bit_brain(publish_configuration()) {}
  life::configuration publish_configuration() {
	life::configuration c;
   	c["size"] = {"8","length of bit brain"};
return c; 
  }
  void mutate() ;
  long update() const ;

};
