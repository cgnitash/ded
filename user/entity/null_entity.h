#pragma once

#include "../../components.h"

#include <algorithm>
#include <vector>

class null_entity {

  //std::string in_sense_  = "in-sense,";
  //std::string out_sense_ = "out-sense,";

public:
  void configure(life::entity_spec)
  {
    //es.configure_input("in-sense", in_sense_);
    //es.configure_output("out-sense", out_sense_);
    //in_sense_  = con["input-tags"]["in-sense"];
    //out_sense_ = con["output-tags"]["out-sense"];
  }
  null_entity() { configure(publish_configuration()); }
  life::entity_spec publish_configuration()
  {
    //life::entity_spec es{"null_entity"};
	//con["parameters"] = nullptr;

    //es.bind_input("in-sense", in_sense_);
    //es.bind_output("out-sense", out_sense_);
    //con["input-tags"]["in-sense"]   = in_sense_;
    //con["output-tags"]["out-sense"] = out_sense_;

    return {};
  }

  void         reset(){}
  void         mutate(){}
  void         input(std::string, life::signal){}
  life::signal output(std::string){ return{}; }
  void         tick(){}
};
