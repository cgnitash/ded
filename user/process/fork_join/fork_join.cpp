
#include "fork_join.hpp"
#include <any>

ded::concepts::Population
    fork_join::evaluate(ded::concepts::Population pop)
{
  std::map<ded::concepts::Substrate, double> scores;
  /*
auto one_pop = one.evaluate(pop);
auto one_vec = one_pop.getAsVector();
// for(auto const &org : one_vec)
// scores[org.getID()] = std::get<double>(org.data.get_value(info_tag_));

auto two_pop = two.evaluate(pop);
auto two_vec = two_pop.getAsVector();
// for(auto const &org : one_vec)
// scores[org.getID()] = std::get<double>(org.data.get_value(info_tag_));

// one and two don't change population
// only data changes
  */

  for (auto &env : envs_)
  {
    auto new_pop = env.evaluate(pop);
    auto new_vec = new_pop.getAsVector();
    for (auto &org : new_vec)
      scores[org] += std::any_cast<double>(org.data.getValue("x"));
  }

  auto vec = pop.getAsVector();
  for (auto &org : vec)
  {
    org.data.setValue("info", scores[org] / envs_.size());
  }
  pop.merge(vec);

  return pop;
}

