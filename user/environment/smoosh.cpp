

#include "smoosh.h"
#include "../../core/utilities.h"

#include <algorithm>
#include <iostream>
#include <vector>

life::population
    smoosh::evaluate(life::population pop)
{

  auto vec = pop.get_as_vector();

  for(auto &org: vec) {

    org.data.set_value(
        wd_tag_,
        ((weight_ * std::get<double>(org.data.get_value(d1_tag_))) +
         ((1.0 - weight_) * std::get<double>(org.data.get_value(d2_tag_)))) /
            2.0);
    org.data.clear(d1_tag_);
    org.data.clear(d2_tag_);
  }

  pop.merge(vec);
  return pop;
}
