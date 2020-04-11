#pragma once

#include "../../components.h"

#include <algorithm>
#include <vector>

class toggle_bit_brain {

  std::string in_ignored_   = "ignored,";
  std::string out_as_true_  = "out-asT,A<double,size>";
  std::string out_as_false_ = "out-asF,A<double,size>";

  size_t         size_ = 8;
  ded::encoding genome_;

public:
  toggle_bit_brain() { configure(publishConfiguration()); }

  void configure(ded::configuration c)
  {
    size_ = c["parameters"]["size"];

    in_ignored_   = c["input-tags"]["ignored"];
    out_as_true_  = c["output-tags"]["out-asT"];
    out_as_false_ = c["output-tags"]["out-asF"];

    // encoding is simply a random start sequence
    genome_.generate(size_);
  }

  ded::configuration publishConfiguration()
  {
    ded::configuration c;
    c["parameters"]["size"] = size_;

    c["input-tags"]["ignored"]  = in_ignored_;
    c["output-tags"]["out-asT"] = out_as_true_;
    c["output-tags"]["out-asF"] = out_as_false_;

    return c;
  }

  void         reset() {}
  void         mutate();
  void         input(std::string, ded::concepts::Signal);
  ded::concepts::Signal output(std::string);
  void         tick();
};
