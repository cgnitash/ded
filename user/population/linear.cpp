
#include "linear.h"
#include "../../core/utilities.h"

#include <algorithm>
#include <vector>
#include <experimental/filesystem>

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

bool linear::found_in_fossils(long n) const {
  auto l = std::lower_bound(
      std::begin(fossils_), std::end(fossils_), n,
      [](auto &org, long n) { return org.first.get_id() < n; });
  return l != std::end(fossils_) && l->first.get_id() == n;
}

bool linear::found_in_pop(long n) const {
  auto l = std::lower_bound(std::begin(pop_), std::end(pop_), n,
                            [](auto &org, long n) { return org.get_id() < n; });
  return l != ranges::end(pop_) && l->get_id() == n;
}

void linear::merge(std::vector<life::entity> v) {

  if (track_lineage_) {

    ranges::sort(v);

    std::vector<life::entity> new_orgs, del_orgs;
    ranges::set_difference(pop_, v, ranges::back_inserter(del_orgs));
    for (auto &org : del_orgs)
      update_tree(org.get_id(), -1);

    ranges::set_difference(v, pop_, ranges::back_inserter(new_orgs));

    for (auto &org : new_orgs) {

      if (!found_in_fossils(org.get_ancestor())) {
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


life::configuration linear::get_stats(long i) const {
  // precondition: "score" must be in org.data
  const auto scores = pop_ | ranges::view::transform([](auto const &org) {
                        return double{org.data["score"]};
                      });

  auto pop_stats_file =
      open_or_append(dir_name_ + "pop.csv", "avg,max,update\n");

  life::configuration con;
  con["max"] = *ranges::max_element(scores);
  con["avg"] = ranges::accumulate(scores, 0.0) / pop_.size();

  pop_stats_file << con["avg"] << "," << con["max"] << "," << i << std::endl;

  return con;
}

void linear::snapshot(long i) const {
  auto snapshot_file =
      open_or_append(dir_name_ + "snapshot_" + std::to_string(i) + ".csv",
                     "id,size,encoding\n");
  for (auto &org : pop_)
    snapshot_file << org.get_id() << "," << org.get_encoding().size() << ","
      << org.get_encoding() << std::endl;
}

std::ofstream linear::open_or_append(std::string file_name,
                                     std::string header) const {
  std::ofstream file; 
  if (!std::experimental::filesystem::exists(file_name
                                             )) {
    file.open(file_name);
    file << header;
  } else
    file.open(file_name, std::ios::app);
  return file;
}

void linear::prune_lineage(long i) {

  if (!track_lineage_)
    return;

  auto lineage_organisms_file =
      open_or_append(dir_name_ + "lineage_organisms.csv",
                     "id,recorded_at,encoding_size,encoding\n");

  auto lineage_file = open_or_append(dir_name_ + "lineage.csv",
                                     "id,recorded_at,on_lod,ancestor_id\n");

  for (auto &org : fossils_) {
    if (org.second == size_ && !found_in_pop(org.first.get_id())) {
      lineage_organisms_file  << org.first.get_id() << "," << i << ","
                   << org.first.get_encoding().size() << "," << org.first.get_encoding()
                   << std::endl;
      lineage_file << org.first.get_id() << "," << i << "," << 1 << ","
                   << org.first.get_ancestor() << std::endl;
    }
    if (!org.second )
      lineage_file << org.first.get_id() << "," << i << "," << 0 << ","
                   << org.first.get_ancestor() << std::endl;
  }

  fossils_.erase(std::remove_if(std::begin(fossils_), std::end(fossils_),
                                [this](auto &fossil) {
                                  return !fossil.second ||
                                         (fossil.second == size_ &&
                                          !found_in_pop(fossil.first.get_id()));
                                }),
                 std::end(fossils_));

  return ;
}

linear::~linear() {
  std::ofstream unrec_file(dir_name_ + "unpruned.csv");
  unrec_file << "id,recorded_at,on_lod,ancestor_id\n";
  for (auto &org : fossils_) {
    unrec_file << org.first.get_id() << ",-1," << 0 << ","
               << org.first.get_ancestor() << std::endl;
  }
}
