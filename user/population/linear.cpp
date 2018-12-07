
#include "linear.h"
#include "../../core/utilities.h"

#include <algorithm>
#include <vector>

std::vector<life::entity> linear::get_as_vector() { return pop_; }

void linear::update_tree(long org_id, int count) {

  auto possible_ancestry_end = std::end(fossils_);
  while (org_id) {
    auto current_org = std::lower_bound(
        std::begin(fossils_), possible_ancestry_end, org_id,
        [](auto &fossil, auto id) { return fossil.first.get_id() < id; });
    if (current_org == possible_ancestry_end ||
        current_org->first.get_id() != org_id)
      break;
    possible_ancestry_end = current_org;
    current_org->second += count;
    org_id = current_org->first.get_ancestor();
  }
}

void linear::merge(std::vector<life::entity> v) {

  if (track_lineage_) {

    util::rv3::sort(v);

    std::vector<life::entity> new_orgs, del_orgs;
    util::rv3::set_difference(pop_, v, util::rv3::back_inserter(del_orgs));
    for (auto &org : del_orgs)
      update_tree(org.get_id(), -1);

    util::rv3::set_difference(v, pop_, util::rv3::back_inserter(new_orgs));

    for (auto &org : new_orgs) {
      if (auto l = std::lower_bound(
              std::begin(fossils_),std::end(fossils_), org.get_ancestor(),
              [](auto &org, long n) { return org.first.get_id() < n; });
          l == util::rv3::end(fossils_) ||
          l->first.get_id() != org.get_ancestor()) {
        std::cout
            << "warning: unknown ancestor - lineage tracking turned off\n";
        fossils_.clear();
        track_lineage_ = false;
		break;
      }
      if (org.get_id() > fossils_.back().first.get_id())
        fossils_.push_back({org, 0});

      update_tree(org.get_id(), 1);
    }
  }
  pop_ = v;
}


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

void linear::snapshot(std::ofstream &o) const {
  for (auto &org : pop_)
    o << org.get_id() << "," << org.get_encoding().size() << ","
      << org.get_encoding() << std::endl;
}

std::vector<life::entity> linear::prune_lineage() {

  std::vector<life::entity> pruned;
  if (!track_lineage_)
    return pruned;

  for (auto &fossil : fossils_) {
    if (fossil.second == size_)
      pruned.push_back(fossil.first);
  }

  fossils_.erase(std::remove_if(std::begin(fossils_), std::end(fossils_),
                                [this](auto &fossil) {
                                  return !fossil.second ||
                                         fossil.second == size_;
                                }),
                 std::end(fossils_));

  return pruned;
}

