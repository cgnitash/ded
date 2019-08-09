
#include "max_one.h"

#include <algorithm>

ded::concepts::Population
    max_one::evaluate(ded::concepts::Population pop)
{
  auto vec = pop.get_as_vector();
  for (auto &org : vec)
  {
    org.reset();
    org.tick();
    org.data.setValue(ones_tag_,
                       ranges::accumulate(std::get<std::vector<double>>(
                                              org.output(org_output_ones_tag_)),
                                          0.0) /
                           length_);
  }
  pop.merge(vec);
  return pop;
}
