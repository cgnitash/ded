
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
  life::entity ent ;
  life::selector sel ;

public:
  evolution() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() {
    life::configuration ec;
    ec["entity"] = {"null_entity", {}, "the entities to be evolved"};
    ec["selector"] = {
        "null_selector", {}, "selection process used by evolution"};
    ec["population_size"] = 100;
    ec["generations"] = 50;
    return ec;
  }

  void configure(life::configuration con) {

    pop_size = (con["population_size"]);
    generations = (con["generations"]);
    ent = life::make_entity(std::string(con["entity"][0]));
    ent.configure(con["entity"][1]);
    sel = life::make_selector(std::string(con["selector"][0]));
    sel.configure(con["selector"][1]);
  }

  void run();
};

