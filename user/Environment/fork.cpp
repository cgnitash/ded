
#include "fork.h"

ded::concepts::Population
    fork::evaluate(ded::concepts::Population pop)
{
  auto one_pop = one.evaluate(pop);
  auto one_vec = one_pop.get_as_vector();

  auto two_pop = two.evaluate(pop);
  auto two_vec = two_pop.get_as_vector();

  // one and two don't change population
  // only data changes
  auto vec = pop.get_as_vector();
  for (auto &org : vec)
  {
    org.data.merge(ranges::find(one_vec, org)->data);
    org.data.merge(ranges::find(two_vec, org)->data);
  }
  pop.merge(vec);

  return pop;
}
