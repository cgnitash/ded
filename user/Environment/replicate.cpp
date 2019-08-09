
#include "replicate.h"
#include <vector>
#include <range/v3/all.hpp>

ded::concepts::Population
    replicate::evaluate(ded::concepts::Population pop)
{
  std::vector accumulated_scores(pop.size(), 0.0);

  ded::utilities::repeat(num_, [&] {
    // evaluate the entire population
    // add scores to accumulated scores
	auto res_pop = env.evaluate(pop);
    ranges::transform(res_pop.get_as_vector(),
                      accumulated_scores,
                      ranges::begin(accumulated_scores),
                      [this](const auto &org, auto score) {
                        return score +
                               std::get<double>(org.data.getValue(x_tag_));
                      });
  });

  auto vec = pop.get_as_vector();

  // assign accumulate scores to each org
  ranges::transform(vec,
                    accumulated_scores,
                    ranges::begin(vec),
                    [this](auto org, const auto score) {
                      org.data.setValue(x_tag_, score / this->num_);
                      return org;
                    });

  pop.merge(vec);
  return pop;
}
