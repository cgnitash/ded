

#include "components.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

template <typename L, typename M>
void save(std::ofstream &ofs, std::string_view comp_type, L component_lister,
          M component_maker) {

  for (auto &comp_name : component_lister())
    ofs << "\n"
        << comp_type << " -- " << comp_name << "\n  " << std::setw(4)
        << component_maker(comp_name).publish_configuration() << std::endl;
}
void save_configs() {

  std::ofstream file("configurations.cfg");

  save(file, "experiment", life::experiment_list, life::make_experiment);
  save(file, "entity", life::entity_list, life::make_entity);
  save(file, "environment", life::environment_list, life::make_environment);
  save(file, "population", life::population_list, life::make_population);
}

life::configuration load_user_experiment(std::string file_name) {
  life::configuration con;
  std::ifstream ifs(file_name);
  ifs >> con;
  return con;
}

long life::entity::entity_id_ = 0;

int main(int argc, char **argv) {
  // TODO use an actual command-line library :P
  //

  if (argc == 2 && std::string(argv[1]) == "-h") {
    std::cout << "-s : saves configuration files\n"
              << "-f <file-name> <seed> : runs experiment in file-name "
                 "with seed\n";
  } else if (argc == 2 && std::string(argv[1]) == "-s") {
    std::cout << "saving configurations.cfg ... \n";
    save_configs();
  } else if (argc == 4 && std::string(argv[1]) == "-f") {
    std::cout << "running experiment from file " << argv[2] << " with seed "
              << argv[3] << " ... \n";
    srand(std::stol(argv[3]));
    auto con = load_user_experiment(argv[2]);
    std::string name = con[0];
    auto exp = life::make_experiment(name);
	con[1]["REP"] = argv[3];
    exp.configure(con[1]);
    exp.run();
  } else {
    std::cout << "ded: unknown command line arguments. try -h\n";
	auto e = life::make_entity("bit_brain");
	e.data["hi"] = 42;
	std::cout << e.data["hi"] << "\n";
	std::cout << e.data["bi"] << "\n";
	auto x { e};
	std::cout << x.data["hi"] << "\n";
	std::cout << x.data["bi"] << "\n";
	auto&y { e};
	std::cout << y.data["hi"] << "\n";
	std::cout << y.data["bi"] << "\n";
  }
}
