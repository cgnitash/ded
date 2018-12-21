

#include "components.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
using ModuleInstancePair = std::pair<std::string, std::string>;
std::map<ModuleInstancePair, life::configuration> all_configs;

template <typename L, typename M>
void generate_config(std::string comp_type, L component_lister,
                     M component_maker) {

  for (auto &comp_name : component_lister())
    all_configs[{comp_type, comp_name}] =
        component_maker(comp_name).publish_configuration();
}

void generate_all_configs() {
  generate_config("experiment", life::experiment_list, life::make_experiment);
  generate_config("entity", life::entity_list, life::make_entity);
  generate_config("environment", life::environment_list,
                  life::make_environment);
  generate_config("population", life::population_list, life::make_population);
}

void save_configs() {

  std::ofstream file("configurations.cfg");
  for (auto &[type_name, config] : all_configs)
    file << "\n"
         << type_name.first << " -- " << type_name.second << "\n  "
         << std::setw(4) << config << std::endl;
}

void config_mismatch_error(std::string key, ModuleInstancePair mip) {

  std::cout << "Error: Configuration mismatch -- <" << mip.first
            << ">::" << mip.second << " does not have parameter named \"" << key
            << "\"\nDid you mean something else?\n";
  std::exit(1);
}

std::string remove_null(std::string s) { return s.substr(5); }

life::configuration true_parameters(ModuleInstancePair mip) {
  auto real_con_it = all_configs.find(mip);
  if (all_configs.end() == real_con_it) {
    std::cout << "Error: Non-existent <Module>::Instance -- <" << mip.first
              << ">::" << mip.second << std::endl;
    std::exit(1);
  }
  return real_con_it->second;
}

void type_error_mismatch(std::string real, std::string fake,
                         ModuleInstancePair mip) {
  std::cout << "Error: Type mismatch -- <" << mip.first << ">::" << mip.second
            << " must have type '" << real << "' but has type '" << fake
            << "'\n";
  std::exit(1);
}

life::configuration true_object(ModuleInstancePair mip,
                                life::configuration con) {

  auto real_con = true_parameters(mip);

  for (auto it = con.begin(); it != con.end(); ++it) {
    auto rit = real_con.find(it.key());
    if (rit == real_con.end())
      config_mismatch_error(it.key(), mip);

    if (rit->type_name() != it->type_name())
      type_error_mismatch(rit->type_name(), it->type_name(), mip);

    rit.value() = it->type_name() == std::string{"array"}
                      ? life::configuration::array(
                            {it.value()[0],
                             true_object({std::string{rit.value()[0]}.substr(5),
                                          it.value()[0]},
                                         it.value()[1])})
                      : it.value();
  }
  return real_con;
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
      {con[0], true_object({"experiment", con[0]}, con[1])});
}

long life::entity::entity_id_ = 0;

std::string life::global_path = "./";

int main(int argc, char **argv) {
  // TODO use an actual command-line library :P
  //

  generate_all_configs();
  std::string exp_path = argv[2];
  life::global_path = exp_path.substr(0, exp_path.find_last_of('/') + 1);

  std::hash<std::string> hash_fn;
  auto con = true_user_experiment(exp_path);
  std::cout << std::setw(4) << con << std::endl;
  std::cout << "\nSuccessfully Tested! Generated unique experiment "
            << hash_fn(con.dump()) << std::endl;

  if (argc == 2 && std::string(argv[1]) == "-h") {
    std::cout << "-s : saves configuration files\n"
              << "-f <file-name> : runs experiment in file-name\n"
              << "-t <file-name> : tests experiment in file-name\n";
  } else if (argc == 2 && std::string(argv[1]) == "-s") {
    std::cout << "saving configurations.cfg ... \n";
    save_configs();
  } else if (argc == 3 && std::string(argv[1]) == "-f") {
    // auto con = true_user_experiment(exp_path);
    std::string name = con[0];
    auto exp = life::make_experiment(name);
    exp.configure(con[1]);
    std::cout << "running experiment from file \"" << exp_path << "\" ... \n";
    exp.run();
    std::cout << "\nSuccessfully Completed!\n";
  } else {
    std::cout << "ded: unknown command line arguments. try -h\n";
    // auto e = life::make_entity("bit_brain");
    // auto b = e;
    // std::cout << (b < e);
  }
}
