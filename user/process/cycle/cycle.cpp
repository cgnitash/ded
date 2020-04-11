
#include "cycle.hpp"


ded::concepts::Population
    cycle::evaluate(ded::concepts::Population pop)
{

  ded::utilities::repeat(count_, [&] { pop = world.evaluate(pop); });

  return pop;
}
