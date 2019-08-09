

#include "elite_replacement.h"

#include <range/v3/all.hpp>

ded::concepts::Population
    elite_replacement::evaluate(ded::concepts::Population populate)
{

  auto pop = populate.get_as_vector();
  // find the strength_ fraction of highest "score"s
  const auto fraction =
      std::max(size_t{ 1 }, static_cast<size_t>(pop.size() * strength_));

  ranges::partial_sort(
      ranges::begin(pop),
      ranges::begin(pop) +  fraction,
      ranges::end(pop),
      [this](const auto &org1, const auto &org2) {
        return std::get<double>(org1.data.getValue(value_tag_)) >
               std::get<double>(org2.data.getValue(value_tag_));
      });

  // done with value_tag_
  for (auto &org : pop) org.data.clear(value_tag_);

  // get the sorted best fraction of orgs from above
  auto best_orgs = ranges::view::take(pop, fraction);

  // make roughly equal mutated copies of each best org
  populate.merge(ranges::view::concat(best_orgs,
                                      best_orgs | ranges::view::cycle |
                                          ranges::view::transform([](auto org) {
                                            org.mutate();
                                            return org;
                                          })) |
                 ranges::view::take(pop.size()));

  populate.prune_lineage(invoke_++);
  return populate;
}
