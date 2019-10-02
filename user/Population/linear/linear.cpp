
#include "linear.h"
#include "../../../core/utilities/csv/csv.h"

#include <algorithm>
#include <experimental/filesystem>
#include <vector>

void
    linear::initialize()
{

  if (load_spec_.empty())
  {
    pop_.clear();
    rs::generate_n(rs::back_inserter(pop_), size_, [&] {
      auto org = ded::makeSubstrate(org_);
      if (track_lineage_) fossils_.push_back({ org, 1 });
      return org;
    });
  } else
  {

    if (!std::experimental::filesystem::exists(ded::GLOBAL_PATH + load_spec_))
    {
      std::cout << "error: path \"" << ded::GLOBAL_PATH + load_spec_
                << "\" does not exist\n";
	  throw ded::specs::SpecError{};
    }
	ded::utilities::csv::CSV  csv(ded::GLOBAL_PATH + load_spec_);
    auto ids = csv.single_column("id");
    rs::sort(ids,
                 [](auto a, auto b) { return std::stol(a) > std::stol(b); });
    pop_.clear();
    rs::transform(ids, rs::back_inserter(pop_), [&](auto id) {
      auto org = ded::makeSubstrate(org_);
      org.setEncoding(org.parseEncoding(csv.look_up("id", id, "encoding")));
      if (track_lineage_) fossils_.push_back({ org, 1 });
      return org;
    });
  }
}

std::vector<ded::concepts::Substrate>
    linear::getAsVector()const 
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
        [](auto &fossil, auto id) { return fossil.first.getID() < id; });
    if (current_org == possible_ancestry_end ||
        current_org->first.getID() != org_id)
      break;
    possible_ancestry_end = current_org;
    current_org->second += count;
    org_id = current_org->first.getAncestor();
  }
}

bool
    linear::found_in_fossils(long n) const
{
  auto l = std::lower_bound(
      std::begin(fossils_), std::end(fossils_), n, [](auto &org, long n) {
        return org.first.getID() < n;
      });
  return l != std::end(fossils_) && l->first.getID() == n;
}

bool
    linear::found_in_pop(long n) const
{
  auto l = std::lower_bound(std::begin(pop_),
                            std::end(pop_),
                            n,
                            [](auto &org, long n) { return org.getID() < n; });
  return l != rs::end(pop_) && l->getID() == n;
}

void
    linear::merge(std::vector<ded::concepts::Substrate> v)
{

  if (track_lineage_)
  {

    rs::sort(v);

    std::vector<ded::concepts::Substrate> new_orgs, del_orgs;
    rs::set_difference(pop_, v, rs::back_inserter(del_orgs));
    for (auto &org : del_orgs) update_tree(org.getID(), -1);

    rs::set_difference(v, pop_, rs::back_inserter(new_orgs));

    for (auto &org : new_orgs)
    {

      if (!found_in_fossils(org.getAncestor()))
      {
        std::cout
            << "warning: unknown ancestor - lineage tracking turned off\n";
        fossils_.clear();
        track_lineage_ = false;
        break;
      }
      if (org.getID() > fossils_.back().first.getID())
        fossils_.push_back({ org, 0 });

      update_tree(org.getID(), 1);
    }
  }
  pop_ = v;
}

void
    linear::snapShot(long i) const
{
  auto snapshot_file = ded::utilities::open_or_append(ded::GLOBAL_PATH + "snapshot_" +
                                                std::to_string(i) + ".csv",
                                            "id,size,encoding\n");
  for (auto &org : pop_)
    snapshot_file << org.getID() << "," << org.getEncoding().size() << ","
                  << org.getEncoding() << std::endl;
}

void
    linear::pruneLineage(long i)
{

  if (!track_lineage_) return;

  auto lineage_organisms_file =
      ded::utilities::open_or_append(ded::GLOBAL_PATH + "lineage_organisms.csv",
                           "id,recorded_at,encoding_size,encoding\n");

  auto lineage_file = ded::utilities::open_or_append(
      ded::GLOBAL_PATH + "lineage.csv", "id,recorded_at,on_lod,ancestor_id\n");

  for (auto &org : fossils_)
  {
    if (org.second == size_ && !found_in_pop(org.first.getID()))
    {
      lineage_organisms_file << org.first.getID() << "," << i << ","
                             << org.first.getEncoding().size() << ","
                             << org.first.getEncoding() << std::endl;
      lineage_file << org.first.getID() << "," << i << "," << 1 << ","
                   << org.first.getAncestor() << std::endl;
    }
    if (!org.second)
      lineage_file << org.first.getID() << "," << i << "," << 0 << ","
                   << org.first.getAncestor() << std::endl;
  }

  fossils_.erase(std::remove_if(std::begin(fossils_),
                                std::end(fossils_),
                                [this](auto &fossil) {
                                  return !fossil.second ||
                                         (fossil.second == size_ &&
                                          !found_in_pop(fossil.first.getID()));
                                }),
                 std::end(fossils_));

  return;
}

void
    linear::flushUnpruned()
{
  pop_.clear();

  // maybe losing some orgs here
  // prune_lineage(-1);
  
  auto unrec_file = ded::utilities::open_or_append(ded::GLOBAL_PATH + "unpruned.csv",
                                         "id,recorded_at,on_lod,ancestor_id\n");
  for (auto &org : fossils_)
  {
    unrec_file << org.first.getID() << ",-1," << 0 << ","
               << org.first.getAncestor() << std::endl;
    //if (org.second == size_) std::cout << org.first.getID();
  }
}
