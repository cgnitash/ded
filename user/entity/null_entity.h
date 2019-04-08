#pragma once

#include "../../components.h"

#include <algorithm>
#include <vector>

class null_entity {

  std::string in_sense_  = "in-sense,";
  std::string out_sense_ = "out-sense,";

public:
  void configure(life::configuration con)
  {
    in_sense_  = con["input-tags"]["in-sense"];
    out_sense_ = con["output-tags"]["out-sense"];
  }
  null_entity() { configure(publish_configuration()); }
  life::configuration publish_configuration()
  {
    life::configuration con;
    con["parameters"] = nullptr;

    con["input-tags"]["in-sense"]   = in_sense_;
    con["output-tags"]["out-sense"] = out_sense_;

    return con;
  }

  void         reset();
  void         mutate();
  void         input(std::string, life::signal);
  life::signal output(std::string);
  void         tick();
};
