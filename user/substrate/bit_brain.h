#pragma once

#include "../../components.h"

#include <algorithm>
#include <vector>

class bit_brain {

  std::string in_sense_  = "in-sense,";
  std::string out_sense_ = "out-sense,A<double,size>";

  size_t         size_ = 8;
  ded::encoding genome_;

public:
  bit_brain() { configure(publishConfiguration()); }

  void configure(ded::configuration c)
  {
    size_ = c["parameters"]["size"];

    in_sense_  = c["input-tags"]["in-sense"];
    out_sense_ = c["output-tags"]["out-sense"];

    // encoding is simply a random start sequence
    genome_.generate(size_);
  }

  ded::configuration publishConfiguration()
  {
    ded::configuration c;
    c["parameters"]["size"] = size_;

    c["input-tags"]["in-sense"]   = in_sense_;
    c["output-tags"]["out-sense"] = out_sense_;

    return c;
  }

  void         reset() {}
  void         mutate();
  void         input(std::string, ded::concepts::Signal);
  ded::concepts::Signal output(std::string);
  void         tick();
};
