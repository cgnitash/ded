
#include "max_one.hpp"

#include <algorithm>
#include <any>

ded::concepts::Population
    max_one::evaluate(ded::concepts::Population pop)
{
  auto vec = pop.getAsVector();
  for (auto &org : vec)
  {
    org.reset();
    org.tick();
    org.data.setValue(
        "ones",
        rs::accumulate(
            std::any_cast<std::vector<double>>(org.output(output_ones)), 0.0) /
            length_);
  }
  pop.merge(vec);
  return pop;
}
