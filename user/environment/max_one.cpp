
#include "max_one.h"
#include "../../core/utilities.h"
#include <algorithm>

life::population max_one::evaluate(life::population pop) {

  pop.merge(pop.get_as_vector() | ranges::action::transform([](auto &org) {
              // no inputs
              // run single tick
              org.tick();
              // score is number of outputs that evaluate to Bit() == 1
              org.data["score"] = ranges::count_if(
                  org.output(), [](auto i) { return util::Bit(i); });
              return org;
            }));
  return pop;
}

