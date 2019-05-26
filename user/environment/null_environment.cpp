
/*
 * only needed for core build system 
 * everything is provided trivially by header
 *
#include "null_environment.h"

life::population
    null_environment::evaluate(life::population pop)
{
  pop.merge(pop.get_as_vector() | ranges::action::transform([](auto &org) {
              org.data.set_value("score", 0.0);
              return org;
            }));
  return pop;
}
*/
