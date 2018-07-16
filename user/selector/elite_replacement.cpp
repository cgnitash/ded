

#include "elite_replacement.h"

#include <algorithm>
#include <iostream>
#include <vector>


  std::vector<life::entity> elite_replacement::select(std::vector<life::entity> &pop) const {
    
	int str = pop.size() * strength_;
    int rem = pop.size() % str;
   
   // get rank of current pop	
	std::nth_element(
        std::begin(pop), std::begin(pop) + str, std::end(pop),
        [](auto &a, auto &b) { return a.score() > b.score(); });

	// new population
    std::vector<life::entity> next(pop);
    for (auto i = 1u; i < next.size() / str; i++)
      std::transform(std::begin(next), std::begin(next) + str,
                     std::begin(next) + i *   str, [](auto const value) {
                       value.mutate();
                       return value;
                     });

    std::transform(std::begin(next), std::begin(next) + rem,
                   std::end(next) - rem, [](auto const value) {
                     value.mutate();
                     return value;
                   });
    return next;
  }

