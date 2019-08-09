
#include "fork.h"

ded::concepts::Population
    fork::evaluate(ded::concepts::Population pop)
{
//	std::map<int, double> scores;
  auto one_pop = one.evaluate(pop);
  auto one_vec = one_pop.get_as_vector();
 // for(auto const &org : one_vec)
   // scores[org.get_id()] = std::get<double>(org.data.get_value(info_tag_));

  auto two_pop = two.evaluate(pop);
  auto two_vec = two_pop.get_as_vector();
 // for(auto const &org : one_vec)
   // scores[org.get_id()] = std::get<double>(org.data.get_value(info_tag_));

  // one and two don't change population
  // only data changes
  auto vec = pop.get_as_vector();
  for (auto &org : vec)
  {
    org.data.setValue(
        info_tag_,
        (std::get<double>(
             ranges::find(one_vec, org)->data.getValue(info_tag_)) +
         std::get<double>(
             ranges::find(two_vec, org)->data.getValue(info_tag_))) /
            2.0);
  }
  pop.merge(vec);

  return pop;
}
