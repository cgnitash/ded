

#include "smoosh.h"
#include "../../core/utilities/utilities.h"

ded::concepts::Population
    smoosh::evaluate(ded::concepts::Population pop)
{

  auto vec = pop.getAsVector();

  for(auto &org: vec) {

    org.data.setValue(
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
