

#include "elite_replacement.hpp"

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
      [](const auto &org1, const auto &org2) {
        return std::any_cast<double>(org1.data.getValue("value")) >
               std::any_cast<double>(org2.data.getValue("value"));
      });

  // get the sorted best fraction of orgs from above
  auto best_orgs = rv::take(pop, fraction);

  // make roughly equal mutated copies of each best org
  populate.merge(
      rv::concat(best_orgs, best_orgs | rv::cycle | rv::transform([](auto org) {
                              org.mutate();
                              return org;
                            })) |
      rv::take(pop.size()) | rs::to<std::vector<ded::concepts::Substrate>>);

  populate.pruneLineage(invoke_++);
  return populate;
}
