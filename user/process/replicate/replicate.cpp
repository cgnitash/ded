
#include "replicate.hpp"
#include <vector>

ded::concepts::Population
    replicate::evaluate(ded::concepts::Population pop)
{
  std::vector accumulated_scores(pop.size(), 0.0);

  ded::utilities::repeat(num_, [&] {
    // evaluate the entire population
    // add scores to accumulated scores
	auto res_pop = env.evaluate(pop);
        rs::transform(res_pop.getAsVector(),
                      accumulated_scores,
                      rs::begin(accumulated_scores),
                      [](const auto &org, auto score) {
                        return score +
                               std::any_cast<double>(org.data.getValue("x"));
                      });
  });

  auto vec = pop.getAsVector();

  // assign accumulate scores to each org
  rs::transform(vec,
                    accumulated_scores,
                    rs::begin(vec),
                    [this](auto org, const auto score) {
                      org.data.setValue("fx", score / this->num_);
                      return org;
                    });

  pop.merge(vec);
  return pop;
}
