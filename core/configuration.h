
# pragma once

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace life{

using configuration =
//    std::map<std::string, std::pair<std::string, std::string>>;
nlohmann::json;

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
}
