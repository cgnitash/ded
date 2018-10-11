

#include "elite_replacement.h"

#include <algorithm>
#include <iostream>
#include <vector>

std::vector<life::entity>
elite_replacement::select(life::eval_results &pop) {

  std::vector<life::entity> result;
  std::string name = "score";
  int frac = pop.size()*strength_;
  std::nth_element(std::begin(pop), std::begin(pop) + frac,
                   std::end(pop), [&name](auto &org1, auto &org2) {
                     return std::stod(org1.second[name]) >
                            std::stod(org2.second[name]);
                   });

  for (auto i = 0; i < frac; i++) {
    auto org = pop[i].first;
    result.push_back(org);
	for (auto j = 0; j < 1/strength_ - 1; j++) {
    org.mutate();
    result.push_back(org);
	}
  }

  result.erase(std::begin(result) + pop.size(),std::end(result));
  if (pop.size() > result.size())
  {
	  std::cout << "elite_replacement error  " << pop.size() << " " << result.size();
	 exit(1);
  } 
  return result;
}

