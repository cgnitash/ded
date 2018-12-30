

#include "components.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

life::configuration true_user_population(std::string file_name) {
  life::configuration con;
  std::ifstream ifs(file_name);
  if (!ifs.is_open()) {
    std::cout << "Error: population file \"" << file_name
              << "\" does not exist\n";
    std::exit(1);
  }
  ifs >> con;
    return con;
}

life::configuration true_user_environment(std::string file_name) {
  life::configuration con;
  std::ifstream ifs(file_name);
  if (!ifs.is_open()) {
    std::cout << "Error: environment file \"" << file_name
              << "\" does not exist\n";
    std::exit(1);
  }
  ifs >> con;
    return con;
}

life::configuration true_user_experiment(std::string file_name) {
  life::configuration con;
  std::ifstream ifs(file_name);
  if (!ifs.is_open()) {
    std::cout << "Error: experiment file \"" << file_name
              << "\" does not exist\n";
    std::exit(1);
  }
  ifs >> con;

  return life::configuration::array(
      {con[0], life::config::true_object({"experiment", con[0]}, con[1])});
}

void save_configs() {

  std::ofstream file("configurations.cfg");
  for (auto &[type_name, config] : life::all_configs)
    file << "\n<"
         << type_name.first << ">::" << type_name.second << "\n  "
         << std::setw(4) << config << std::endl;
}

long life::entity::entity_id_ = 0;

std::string life::global_path = "./";

std::map<life::ModuleInstancePair, life::configuration> life::all_configs;

int main(int argc, char **argv) {
  // TODO use an actual command-line library :P
  //

  life::generate_all_configs();

  if (argc == 2 && std::string(argv[1]) == "-h") {
    std::cout << "-s : saves configuration files\n"
              << "-f <file-name> : runs experiment in file-name\n";
  } else if (argc == 2 && std::string(argv[1]) == "-s") {
    std::cout << "saving configurations.cfg ... \n";
    save_configs();
  } else if (argc == 3 && std::string(argv[1]) == "-f") {
    std::string exp_path = argv[2];
    life::global_path = exp_path.substr(0, exp_path.find_last_of('/') + 1);

    std::hash<std::string> hash_fn;
    auto con = true_user_experiment(exp_path);
    std::cout << std::setw(4) << con << std::endl;
    std::cout << "\nSuccessfully Tested! Generated unique experiment "
              << hash_fn(con.dump()) << std::endl;

    // auto con = true_user_experiment(exp_path);
    std::string name = con[0];
    auto exp = life::make_experiment(name);
    exp.configure(con[1]);
    std::cout << "running experiment from file \"" << exp_path << "\" ... \n";
    exp.run();
    std::cout << "\nSuccessfully Completed!\n";
  } else if (argc == 3 && std::string(argv[1]) == "-g") {
    std::string exp_path = argv[2];
    life::global_path = exp_path.substr(0, exp_path.find_last_of('/') + 1);
    auto gen = life::make_experiment("generator");
    std::cout << "generating sub-experiments from file \"" << exp_path << "\" ... \n";
	life::configuration con;
	con["file"] = exp_path;
    gen.configure(con);
    gen.run();
    std::cout << "\nSuccessfully Generated!\n";

  } else if (argc == 4 && std::string(argv[1]) == "-t") {
    std::string pop_exp_path = argv[2];
    life::global_path = pop_exp_path.substr(0, pop_exp_path.find_last_of('/') + 1);
    std::string env_exp_path = argv[3];
	std::string env_path = env_exp_path.substr(0, env_exp_path.find_last_of('/') + 1);
	if( env_path != life::global_path) {
    	std::cout << "error: both pop and env must come from same dir\n";
		std::exit(1);
	}
    std::hash<std::string> hash_fn;

    auto pop_con = true_user_population(pop_exp_path);
    std::cout << std::setw(4) << pop_con << std::endl;
    std::cout << "\nNot yet Tested! Generated unique population "
              << hash_fn(pop_con.dump()) << std::endl;

    std::string pop_name = pop_con[0];
    auto pop = life::make_population(pop_name);
    pop.configure(pop_con[1]);
    std::cout << "loaded experiment with population from file \"" << pop_exp_path << "\" ... \n";

    auto env_con = true_user_environment(env_exp_path);
    std::cout << std::setw(4) << env_con << std::endl;
    std::cout << "\nNot yet Tested! Generated unique environment "
              << hash_fn(env_con.dump()) << std::endl;

    std::string env_name = env_con[0];
    auto env = life::make_environment(env_name);
    env.configure(env_con[1]);
    std::cout << "loaded experiment with environment from file \"" << env_exp_path << "\" ... \n";

	auto res_pop = env.evaluate(pop);


    std::cout << "\nYay?? Ran succesfully?\n";

  } else {
    std::cout << "ded: unknown command line arguments. try -h\n";
    // auto e = life::make_entity("bit_brain");
    // auto b = e;
    // std::cout << (b < e);
  }
}
