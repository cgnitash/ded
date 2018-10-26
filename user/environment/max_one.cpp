
#include "max_one.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

void max_one::evaluate(std::vector<life::entity> &pop) {

  for (auto &org : pop) {
    org.tick();
    auto out = org.output();
	  org.data["score"] = 
    std::count(std::begin(out), std::end(out), 1);
  }

          /*
  std::transform(std::begin(pop), std::end(pop), std::back_inserter(scores),
                 [](auto &org) {
                   std::map<std::string, std::string> m;
                   auto o = org;
				   o.tick();
                   auto v = o.output();
                   m["score"] = std::to_string(
                       std::count(std::begin(v), std::end(v), 1));
                   return std::make_pair(o, m);
                 });
				 */
}

