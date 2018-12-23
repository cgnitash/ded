
# pragma once

#include "nlohmann/json.hpp"
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>

namespace life{

using configuration = nlohmann::json;
using ModuleInstancePair = std::pair<std::string, std::string>;
extern std::map<ModuleInstancePair, life::configuration> all_configs;

inline void validate_subset(const configuration &in, const configuration &real) {
    for (auto it = in.begin(); it != in.end(); ++it) {
      if (real.find(it.key()) == real.end()) {
        std::cout << "Error: Configuration mismatch -- \"" << it.key()
                  << "\" is not a valid parameter\n";
        exit(1);
      }
    }
}

inline void merge_into(configuration &in, const configuration &real) {
  for (auto it = real.begin(); it != real.end(); ++it) {
    if (in.find(it.key()) == in.end()) {
      in[it.key()] = it.value();
    }
  }
}
} // namespace life
