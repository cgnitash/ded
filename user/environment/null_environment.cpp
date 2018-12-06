
#include "null_environment.h"

life::population null_environment::evaluate(life::population pop) {
  pop.merge(pop.get_as_vector() | util::rv3::action::transform([](auto &org) {
              org.data["score"] = 0.0;
              return org;
            }));
  return pop;
}

