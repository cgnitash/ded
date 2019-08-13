
#include "sequence.h"

ded::concepts::Population
    sequence::evaluate(ded::concepts::Population pop)
{


  pop = first.evaluate(pop);

  pop = second.evaluate(pop);

  return pop;
}
