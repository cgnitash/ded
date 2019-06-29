
#include "linear.h"
#include "../../core/utilities/csv/csv.h"

#include <algorithm>
#include <experimental/filesystem>
#include <vector>

void
    linear::initialize()
{

  if (load_spec_.empty())
  {
    pop_.clear();
    ranges::generate_n(ranges::back_inserter(pop_), size_, [&] {
      auto org = ded::make_Entity(org_);
      if (track_lineage_) fossils_.push_back({ org, 1 });
      return org;
    });
  } else
  {

    if (!std::experimental::filesystem::exists(ded::global_path + load_spec_))
    {
      std::cout << "error: path \"" << ded::global_path + load_spec_
                << "\" does not exist";
      std::exit(1);
    }
	ded::utilities::csv::CSV  csv(ded::global_path + load_spec_);
    auto ids = csv.single_column("id");
    ranges::sort(ids,
                 [](auto a, auto b) { return std::stol(a) > std::stol(b); });
    pop_.clear();
    ranges::transform(ids, ranges::back_inserter(pop_), [&](auto id) {
      auto org = ded::make_Entity(org_);
      org.set_encoding(org.parse_encoding(csv.look_up("id", id, "encoding")));
      if (track_lineage_) fossils_.push_back({ org, 1 });
      return org;
    });
  }
}

std::vector<ded::concepts::Entity>
    linear::get_as_vector()const 
{
  return pop_;
}

void
    linear::update_tree(long org_id, int count)
{

  auto possible_ancestry_end = std::end(fossils_);
  while (org_id)
  {
    auto current_org = std::lower_bound(
        std::begin(fossils_),
        possible_ancestry_end,
        org_id,
        [](auto &fossil, auto id) { return fossil.first.get_id() < id; });
    if (current_org == possible_ancestry_end ||
        current_org->first.get_id() != org_id)
      break;
    possible_ancestry_end = current_org;
    current_org->second += count;
    org_id = current_org->first.get_ancestor();
  }
}

bool
    linear::found_in_fossils(long n) const
{
  auto l = std::lower_bound(
      std::begin(fossils_), std::end(fossils_), n, [](auto &org, long n) {
        return org.first.get_id() < n;
      });
  return l != std::end(fossils_) && l->first.get_id() == n;
}

bool
    linear::found_in_pop(long n) const
{
  auto l = std::lower_bound(std::begin(pop_),
                            std::end(pop_),
                            n,
                            [](auto &org, long n) { return org.get_id() < n; });
  return l != ranges::end(pop_) && l->get_id() == n;
}

void
    linear::merge(std::vector<ded::concepts::Entity> v)
{

  if (track_lineage_)
  {

    ranges::sort(v);

    std::vector<ded::concepts::Entity> new_orgs, del_orgs;
    ranges::set_difference(pop_, v, ranges::back_inserter(del_orgs));
    for (auto &org : del_orgs) update_tree(org.get_id(), -1);

    ranges::set_difference(v, pop_, ranges::back_inserter(new_orgs));

    for (auto &org : new_orgs)
    {

      if (!found_in_fossils(org.get_ancestor()))
      {
        std::cout
            << "warning: unknown ancestor - lineage tracking turned off\n";
        fossils_.clear();
        track_lineage_ = false;
        break;
      }
      if (org.get_id() > fossils_.back().first.get_id())
        fossils_.push_back({ org, 0 });

      update_tree(org.get_id(), 1);
    }
  }
  pop_ = v;
}

void
    linear::snapshot(long i) const
{
  auto snapshot_file = ded::utilities::open_or_append(ded::global_path + "snapshot_" +
                                                std::to_string(i) + ".csv",
                                            "id,size,encoding\n");
  for (auto &org : pop_)
    snapshot_file << org.get_id() << "," << org.get_encoding().size() << ","
                  << org.get_encoding() << std::endl;
}

void
    linear::prune_lineage(long i)
{

  if (!track_lineage_) return;

  auto lineage_organisms_file =
      ded::utilities::open_or_append(ded::global_path + "lineage_organisms.csv",
                           "id,recorded_at,encoding_size,encoding\n");

  auto lineage_file = ded::utilities::open_or_append(
      ded::global_path + "lineage.csv", "id,recorded_at,on_lod,ancestor_id\n");

  for (auto &org : fossils_)
  {
    if (org.second == size_ && !found_in_pop(org.first.get_id()))
    {
      lineage_organisms_file << org.first.get_id() << "," << i << ","
                             << org.first.get_encoding().size() << ","
                             << org.first.get_encoding() << std::endl;
      lineage_file << org.first.get_id() << "," << i << "," << 1 << ","
                   << org.first.get_ancestor() << std::endl;
    }
    if (!org.second)
      lineage_file << org.first.get_id() << "," << i << "," << 0 << ","
                   << org.first.get_ancestor() << std::endl;
  }

  fossils_.erase(std::remove_if(std::begin(fossils_),
                                std::end(fossils_),
                                [this](auto &fossil) {
                                  return !fossil.second ||
                                         (fossil.second == size_ &&
                                          !found_in_pop(fossil.first.get_id()));
                                }),
                 std::end(fossils_));

  return;
}

void
    linear::flush_unpruned()
{
  pop_.clear();

  // maybe losing some orgs here
  // prune_lineage(-1);
  
  auto unrec_file = ded::utilities::open_or_append(ded::global_path + "unpruned.csv",
                                         "id,recorded_at,on_lod,ancestor_id\n");
  for (auto &org : fossils_)
  {
    unrec_file << org.first.get_id() << ",-1," << 0 << ","
               << org.first.get_ancestor() << std::endl;
    if (org.second == size_) std::cout << org.first.get_id();
  }
}
