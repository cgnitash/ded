
#include "linear.h"
#include "../../core/utilities.h"

#include <algorithm>
#include <vector>

std::vector<life::entity> linear::get_as_vector() { return pop_; }

void linear::merge(std::vector<life::entity> v) {
  pop_.insert(pop_.end(), std::make_move_iterator(std::begin(v)),
              std::make_move_iterator(std::end(v)));
  bottled_.insert(std::begin(pop_), std::end(pop_));
}

void linear::clear() { pop_.clear(); }

life::configuration linear::get_stats() {

  // precondition: "score" must be in org.data
  life::configuration con;
  const auto scores = pop_ | util::rv3::view::transform([](auto const &org) {
                        return double{org.data["score"]};
                      });

  con["avg"] = util::rv3::accumulate(scores, 0.0) / pop_.size();

  con["max"] = *util::rv3::max_element(scores);

  return con;
}

std::vector<life::entity> linear::prune_lineage() {

  std::vector<life::entity> pruned;

  while (true) {
    const auto fronts =
        pop_ | util::rv3::view::transform(&life::entity::get_ancestor_list) |
        util::rv3::view::transform(
            [](auto anc) { return anc.empty() ? 0 : anc.front(); });

    const auto val = util::rv3::front(fronts);

    if (!val || util::rv3::any_of(fronts, [val](long i) { return i != val; }))
      break;

    const auto pos = util::rv3::lower_bound(bottled_, val, util::rv3::less(),
                                            &life::entity::get_id);

    pruned.push_back(*pos);
    bottled_.erase(std::begin(bottled_), pos);

    for (auto &org : pop_)
      org.prune_ancestors(1);
  }

  return pruned;
}

