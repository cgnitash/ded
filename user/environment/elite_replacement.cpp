

#include "elite_replacement.h"
#include "../../core/utilities.h"

#include <algorithm>
#include <iostream>
#include <vector>

life::population elite_replacement::evaluate(life::population populate) {

  // precondition: all entities have field"score" in data
  std::string name = "score";

  auto pop = populate.get_as_vector();
  // find the strength_ fraction of highest "score"s
  const auto fraction = pop.size() * strength_;

  ranges::partial_sort(
      ranges::begin(pop),
      ranges::begin(pop) +
          std::max(size_t{1}, static_cast<size_t>(fraction)),
      ranges::end(pop), [&name](const auto &org1, const auto &org2) {
        return org1.data[name] > org2.data[name];
      });

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

  return populate;
}

