
#pragma once 

#include "../../core/configuration.h"
#include "../../core/entity.h"
#include "../../core/eval_results.h"

#include <algorithm>
#include <iostream>
#include <vector>


class elite_replacement {

	std::string expr_;
public:
  void configure(life::configuration c) { expr_  =c["expr"]; }

life::configuration publish_configuration() const  {
  life::configuration c;
  c["expr"] = "max $score";
  return c;
}
  elite_replacement() { configure(publish_configuration()); }

  std::vector<life::entity> select(const std::vector<life::entity> &,
                                   const life::eval_results &);
};
