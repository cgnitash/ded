

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



//using ModuleInstancePair = std::pair<std::string, std::string>;
//std::map<ModuleInstancePair, life::configuration> all_configs;
/*
 *
 *
auto missing_module_instance_error(life::ModuleInstancePair mip) {
  auto &true_mod = mip.first;
  auto &attempted_inst = mip.second;
  std::cout << "Error: Non-existent <Module>::Instance -- \033[31m<" << true_mod
            << ">::" << attempted_inst << "\033[0m\n";
  // for ([[maybe_unused]] auto &[type_name, config] : all_configs)
  for (auto &type_name_config_pair : life::all_configs) {
    auto &[mod, inst] = type_name_config_pair.first;
    if (mod == true_mod &&
        ranges::any_of(ranges::view::single(inst) | util::all_edits() |
                           util::all_edits(),
                       [=](auto i) { return i == attempted_inst; }))
      std::cout << "Did you mean \033[32m'" << inst << "'\033[0m?\n";
  }
  std::exit(1);
}

life::configuration true_parameters(life::ModuleInstancePair mip) {
  auto real_con_it = life::all_configs.find(mip);
  if (life::all_configs.end() == real_con_it)
	 missing_module_instance_error(mip); 
  return real_con_it->second;
}

void config_mismatch_error(std::string key, life::ModuleInstancePair mip) {

  std::cout << "Error: Configuration mismatch -- \033[33m<" << mip.first
            << ">::" << mip.second
            << "\033[0m does not have parameter named \033[31m'" << key
            << "'\033[0m\n";
  auto con = true_parameters(mip);
  for (auto it = con.begin(); it != con.end(); ++it)
    if (ranges::any_of(ranges::view::single(key) | util::all_edits() |
                           util::all_edits(),
                       [=](auto s) { return s == it.key(); }))
      std::cout << "Did you mean \033[32m'" << it.key() << "'\033[0m?\n";

  std::exit(1);
}

void type_mismatch_error(std::string real, std::string fake,
                         life::ModuleInstancePair mip) {
  std::cout << "Error: Type mismatch -- \033[33m<" << mip.first
            << ">::" << mip.second << "\033[0m must have type \033[32m'"
            << real << "'\033[0m but has type \033[31m'" << fake
            << "'\033[0m\n";
  std::exit(1);
}

life::configuration true_object(life::ModuleInstancePair mip,
                                life::configuration con) {

  auto real_con = true_parameters(mip);

  for (auto it = con.begin(); it != con.end(); ++it) {
    auto rit = real_con.find(it.key());
    if (rit == real_con.end())
      config_mismatch_error(it.key(), mip);

    if (rit->type_name() != it->type_name())
      type_mismatch_error(rit->type_name(), it->type_name(), mip);

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

*/


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

  } else {
    std::cout << "ded: unknown command line arguments. try -h\n";
    // auto e = life::make_entity("bit_brain");
    // auto b = e;
    // std::cout << (b < e);
  }
}
