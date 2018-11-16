
#include"linear.h"
#include"../../core/utilities.h"

#include <vector>
#include <algorithm>

std::vector<life::entity> linear::get_as_vector() {
  return pop_;
}

void linear::merge(std::vector<life::entity> v) {
  pop_.insert(pop_.end(), std::make_move_iterator(std::begin(v)),
              std::make_move_iterator(std::end(v)));
}

void linear::clear() { pop_.clear(); }

life::configuration linear::get_stats() {

  life::configuration con;
  const auto scores = pop_ | util::rv3::view::transform([](auto const &org) {
                        return double{org.data["score"]};
                      });

  con["avg"] = util::rv3::accumulate(scores, 0.0) / pop_.size();

  con["max"] = *util::rv3::max_element(scores);

  // con["ids"] = pop_ | util::rv3::view::transform(
  //                        [](auto const &org) { return long{org.get_id()}; });

  return con;
}
