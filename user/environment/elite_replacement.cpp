

#include "elite_replacement.h"
#include "../../core/utilities.h"

#include <algorithm>
#include <iostream>
#include <vector>

std::vector<life::entity>
elite_replacement::evaluate(const std::vector<life::entity> &pop) {

  // precondition: all entities have field"score" in data
  std::string name = "score";

  // find the strength_ fraction of highest "score"s
  auto temp_pop = pop;
  const auto fraction = temp_pop.size() * strength_;

  util::rv3::nth_element(
      util::rv3::begin(temp_pop), util::rv3::begin(temp_pop) + fraction,
      util::rv3::end(temp_pop), [&name](const auto &org1, const auto &org2) {
        return org1.data[name] > org2.data[name];
      });

  // get the best orgs from above
  auto best_orgs = util::rv3::view::take(temp_pop, fraction);

  // make roughly equal mutated copies of each best org
  return util::rv3::view::concat(best_orgs,
                                 best_orgs | util::rv3::view::cycle |
                                     util::rv3::view::transform([](auto org) {
                                       org.mutate();
                                       return org;
                                     })) |
         util::rv3::view::take(pop.size());
}

