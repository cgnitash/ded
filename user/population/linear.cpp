
#include "linear.h"

#include <algorithm>
#include <vector>
#include <experimental/filesystem>

void linear::initialize() {

  if (load_spec_.empty()) {
    pop_.clear();
    ranges::generate_n(ranges::back_inserter(pop_), size_, [&] {
      auto org = life::make_entity(entity_name_);
      org.configure(entity_config_);
      if (track_lineage_)
        fossils_.push_back({org, 1});
      return org;
    });
  } else {

    if (!std::experimental::filesystem::exists(life::global_path + load_spec_)) {
      std::cout << "error: path \"" << life::global_path + load_spec_ << "\" does not exist";
      std::exit(1);
    }
    CSV csv(life::global_path + load_spec_);
    auto ids = csv.singleColumn("id");
    ranges::sort(ids,
                 [](auto a, auto b) { return std::stol(a) > std::stol(b); });
    pop_.clear();
    ranges::transform(ids, ranges::back_inserter(pop_), [&](auto id) {
      auto org = life::make_entity(entity_name_);
      org.configure(entity_config_);
      org.set_encoding(
          org.parse_encoding(csv.lookUp("id", id, "encoding")));
      if (track_lineage_)
        fossils_.push_back({org, 1});
      return org;
    });
  }
}

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
                        return double{org.data.get_value("score")};
                      });

  auto pop_stats_file =
      util::open_or_append(life::global_path + "pop.csv", "avg,max,update\n");

  life::configuration con;
  con["max"] = *ranges::max_element(scores);
  con["avg"] = ranges::accumulate(scores, 0.0) / pop_.size();

  pop_stats_file << con["avg"] << "," << con["max"] << "," << i << std::endl;

  return con;
}

void linear::snapshot(long i) const {
  if (snapshot_frequency_ && !(i % snapshot_frequency_)) {
    auto snapshot_file = util::open_or_append(life::global_path + "snapshot_" +
                                                  std::to_string(i) + ".csv",
                                              "id,size,encoding\n");
    for (auto &org : pop_)
      snapshot_file << org.get_id() << "," << org.get_encoding().size() << ","
                    << org.get_encoding() << std::endl;
  }
}

void linear::prune_lineage(long i) {

  if (!track_lineage_)
    return;

  auto lineage_organisms_file =
      util::open_or_append(life::global_path + "lineage_organisms.csv",
                     "id,recorded_at,encoding_size,encoding\n");

  auto lineage_file = util::open_or_append(life::global_path + "lineage.csv",
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

void linear::flush_unpruned() {
  pop_.clear();
  prune_lineage(-1);
  auto unrec_file = util::open_or_append(life::global_path + "unpruned.csv",
                                         "id,recorded_at,on_lod,ancestor_id\n");
  for (auto &org : fossils_) {
    unrec_file << org.first.get_id() << ",-1," << 0 << ","
               << org.first.get_ancestor() << std::endl;
	if (org.second == size_) std::cout << org.first.get_id();
  }
}
