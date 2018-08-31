# pragma once

#include"../../core/configuration.h"

#include <vector>
#include <algorithm>


class bit_brain {

  long size_;
  std::vector<bool> encoding_;

public:
  void configure(life::configuration c)
      { size_=c["size"];
        encoding_=std::vector<bool>(size_, false); }

  bit_brain() { configure(publish_configuration()) ;}
  life::configuration publish_configuration() {
	life::configuration c;
   	c["size"] = 8;
return c; 
  }
  void mutate() ;
  long update() const ;

};
