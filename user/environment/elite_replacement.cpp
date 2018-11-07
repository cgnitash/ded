

#include "elite_replacement.h"
#include "../../core/utilities.h"

#include <algorithm>
#include <iostream>
#include <vector>

std::vector<life::entity>
elite_replacement::evaluate(std::vector<life::entity> pop) {

  // precondition: all entities have field"score" in data
  std::string name = "score";

  // find the strength_ fraction of highest "score"s
  const auto fraction = pop.size() * strength_;

  util::rv3::partial_sort(
      util::rv3::begin(pop), util::rv3::begin(pop) + fraction,
      util::rv3::end(pop), [&name](const auto &org1, const auto &org2) {
        return org1.data[name] > org2.data[name];
      });

  // get the sorted best fraction of orgs from above
  auto best_orgs = util::rv3::view::take(pop, fraction);

  // make roughly equal mutated copies of each best org
  return util::rv3::view::concat(best_orgs,
                                 best_orgs | util::rv3::view::cycle |
                                     util::rv3::view::transform([](auto org) {
                                       org.mutate();
                                       return org;
                                     })) |
         util::rv3::view::take(pop.size());
}

