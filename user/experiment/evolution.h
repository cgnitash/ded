
#include"../../core/configuration.h"
#include"../../components.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>
#include <random>
#include <regex>


class evolution {

  long pop_size;
  long generations;
  std::string entity_name;
  std::string selector_name;

public:
  evolution() : evolution(publish_configuration()) {}

  life::configuration publish_configuration() {
    life::configuration ec;
    ec["entity"] = {"bit_brain", "just a bitty brain"};
    ec["selector"] = {"moran", "simplish moran process"};
    ec["population_size"] = {"100", "pop size"};
    ec["generations"] = {"50", "number of generations"};
    return ec;
  }

  evolution(life::configuration con)
      : pop_size(std::stol(con["population_size"].first)),
        generations(std::stol(con["generations"].first)),
        entity_name(con["entity"].first), selector_name(con["selector"].first) {
  }

  void run() ;

};

