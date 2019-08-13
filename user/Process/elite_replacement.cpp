

#include "elite_replacement.h"

ded::concepts::Population
    elite_replacement::evaluate(ded::concepts::Population populate)
{

  auto pop = populate.getAsVector();
  // find the strength_ fraction of highest "score"s
  const auto fraction =
      std::max(size_t{ 1 }, static_cast<size_t>(pop.size() * strength_));

  rs::partial_sort(
      rs::begin(pop),
      rs::begin(pop) +  fraction,
      rs::end(pop),
      [this](const auto &org1, const auto &org2) {
        return std::get<double>(org1.data.getValue(value_tag_)) >
               std::get<double>(org2.data.getValue(value_tag_));
      });

  // done with value_tag_
  for (auto &org : pop) org.data.clear(value_tag_);

  // get the sorted best fraction of orgs from above
  auto best_orgs = rv::take(pop, fraction);

  // make roughly equal mutated copies of each best org
  populate.merge(rv::concat(best_orgs,
                                      best_orgs | rv::cycle |
                                          rv::transform([](auto org) {
                                            org.mutate();
                                            return org;
                                          })) |
                 rv::take(pop.size()));

  populate.pruneLineage(invoke_++);
  return populate;
}
