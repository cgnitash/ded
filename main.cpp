

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

void check_env_correct(life::ModuleInstancePair type_name,
                       life::configuration config) {
  if (config.find("parameters") == config.end()) {
    std::cout << "error: all user modules must provide 'parameters' group\n"
              << type_name.first << ">::" << type_name.second << "\n";
    exit(1);
  }

  if (config.find("pre-tags") == config.end()) {
    std::cout << "error: all user modules must provide 'pre-tags' group\n"
              << type_name.first << ">::" << type_name.second << "\n";
    exit(1);
  }

  if (config.find("post-tags") == config.end()) {
    std::cout << "error: all user modules must provide 'post-tags' group\n"
              << type_name.first << ">::" << type_name.second << "\n";
    exit(1);
  }

  if (config.size() != 3) {
    std::cout << "error: module" << type_name.first << ">::" << type_name.second
              << " has extra keys\n";
    exit(1);
  }
  return;
}

/*
void missing_pre_tag_error(std::string pre_tag, life::ModuleInstancePair mip) {

  std::cout << "error: " << pre_tag << " does not override any pre-tags of "
            << mip.first << ">::" << mip.second << "\n";
  exit(1);
}

void extra_pre_tag_error(std::string pre_tag, life::ModuleInstancePair mip) {

  std::cout << "error: pre-tag " << pre_tag << " needs to be over-ridden explicitly\n" 
            << mip.first << ">::" << mip.second << "\n";
  exit(1);
}

void missing_post_tag_error(std::string post_tag, life::ModuleInstancePair mip) {

  std::cout << "error: " << post_tag << " does not override any post-tags of "
            << mip.first << ">::" << mip.second << "\n";
  exit(1);
}

void extra_post_tag_error(std::string post_tag, life::ModuleInstancePair mip) {

  std::cout << "error: post-tag " << post_tag << " needs to be over-ridden explicitly\n" 
            << mip.first << ">::" << mip.second << "\n";
  exit(1);
}
*/

life::configuration check_pre_tag_overrides(std::string name, std::string nested_name,
                              life::configuration reqs,
                              life::configuration overs,
                              life::configuration published) {

  std::cout << "in pre " << reqs << " " << reqs.size() << "\n"
            << published << " " << published.size() << "\n";
  if (reqs.size() < published.size()) {
    std::cout << "error: environment::" << name
              << " cannot handle all the tags supplied by environment::"
              << nested_name << "\n";
    exit(1);
  }
  if (reqs.size() > published.size()) {
    std::cout << "error: environment::" << nested_name
              << " does not provide sufficient tags needed by environment::"
              << name << "\n";
    exit(1);
  }

  if (reqs.empty())
	  return nullptr;

  if (reqs.size() > 1) {

    std::cout << "oops: ded does not support multiple pre-tag requirements \n";
        exit(1);
  }
  auto attempted_over = published;
  attempted_over.begin().value() = reqs[0];
  if (!overs.empty() && attempted_over != overs) {
    std::cout << "error: explicit overrides of pre-tags " << overs
              << " is invalid.\nNote that an automatic application of "
              << attempted_over << " is available\n";
    exit(1);
  }

  return attempted_over;

}

life::configuration check_post_tag_overrides(std::string name, std::string nested_name,
                              life::configuration reqs,
                              life::configuration overs,
                              life::configuration published) {

  std::cout << "in post " << reqs << " " << reqs.size() << "\n"
            << published << " " << published.size() << "\n";
  if (reqs.size() < published.size()) {
    std::cout << "error: environment::" << name
              << " cannot handle all the tags supplied by environment::"
              << nested_name << "\n";
    exit(1);
  }
  if (reqs.size() > published.size()) {
    std::cout << "error: environment::" << nested_name
              << " does not provide sufficient tags needed by environment::"
              << name << "\n";
    exit(1);
  }

  if (reqs.empty())
	  return nullptr;

  if (reqs.size() > 1) {

    std::cout << "oops: ded does not support multiple post-tag requirements \n";
        exit(1);
  }

  auto attempted_over = published;
  attempted_over.begin().value() = reqs[0];
  if (!overs.empty() && attempted_over != overs) {
    std::cout << "error: explicit overrides of post-tags " << overs
              << " is invalid.\nNote that an automatic application of "
              << attempted_over << " is available\n";
    exit(1);
  }

  return attempted_over;
}

/*
      //check_both_tag_overrides(it.key(), it.value(), rit.value());
void check_both_tag_overrides(std::string name, life::configuration provided,
                         life::configuration reqs) {
  auto nested_con =
      life::config::true_parameters({"environment", provided[0]});

  //std::cout << "<environment>::" << name << " --- tags\n";
  check_post_tag_overrides(name,provided[0],reqs[3], provided[1]["post-tags"],
                           nested_con["post-tags"]);
  check_pre_tag_overrides(name,provided[0],reqs[2], provided[1]["pre-tags"],
                          nested_con["pre-tags"]);
  //std::cout << "\n<environment>::" << name << " --- tags\n";
}
*/

life::configuration true_environment_object(life::ModuleInstancePair mip,
                                life::configuration con) {

  auto real_con = life::config::true_parameters(mip);

  check_env_correct(mip,con);

  life::configuration attempted_pre_tags;
  /*
  for (auto it = con["pre-tags"].begin(); it != con["pre-tags"].end(); it++) {
    auto rit = real_con["pre-tags"].find(it.key());
    if (rit == real_con["pre-tags"].end())
      missing_pre_tag_error(it.key(), mip);
	attempted_pre_tags[it.key()] = rit.value();
  }
  for (auto it = real_con["pre-tags"].begin(); it != real_con["pre-tags"].end(); it++) {
    auto rit = con["pre-tags"].find(it.key());
    if (rit == con["pre-tags"].end())
	  extra_pre_tag_error(it.key(),mip);
  }

  life::configuration attempted_post_tags;
  for (auto it = con["post-tags"].begin(); it != con["post-tags"].end(); it++) {
    auto rit = real_con["post-tags"].find(it.key());
    if (rit == real_con["post-tags"].end())
      missing_post_tag_error(it.key(), mip);
	attempted_post_tags[it.key()] = rit.value();
  }
  for (auto it = real_con["post-tags"].begin(); it != real_con["post-tags"].end(); it++) {
    auto rit = con["post-tags"].find(it.key());
    if (rit == con["post-tags"].end())
	  extra_post_tag_error(it.key(),mip);
  }
  */
  for (auto it = con["parameters"].begin(); it != con["parameters"].end();
       it++) {
    auto rit = real_con["parameters"].find(it.key());
    if (rit == real_con["parameters"].end())
      life::config::config_mismatch_error(it.key(), mip);

    if (rit->type_name() != it->type_name())
      life::config::type_mismatch_error(it.key(), rit->type_name(), it->type_name(), mip);

    if (it->type_name() == std::string{"array"} &&
        rit.value()[0] == "null_environment") {
  //    check_both_tag_overrides(it.key(), it.value(), rit.value());
  auto nested_con =
      life::config::true_parameters({"environment", it.value()[0]});
  it.value()[1]["pre-tags"] = check_pre_tag_overrides(
      it.key(), it.value()[0], rit.value()[2], it.value()[1]["pre-tags"],
      nested_con["pre-tags"]);
  it.value()[1]["post-tags"] = check_post_tag_overrides(
      it.key(), it.value()[0], rit.value()[3], it.value()[1]["post-tags"],
      nested_con["post-tags"]);
  rit.value() = life::configuration::array(
      {it.value()[0],
       true_environment_object({"environment", it.value()[0]}, it.value()[1])});
    } else {
      rit.value() = it.value();
    }
  }
//void check_both_tag_overrides(std::string name, life::configuration provided,
//                         life::configuration reqs) {
//  check_pre_tag_overrides(name,provided[0],reqs[2], provided[1]["pre-tags"],
//                          nested_con["pre-tags"]);
  return real_con;
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
  return life::configuration::array(
      {con[0], true_environment_object({"environment", con[0]}, con[1])});
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
  for (auto &[type_name, config] : life::all_configs) {
    if (type_name.first == "environment") 
		check_env_correct(type_name,config);

    file << "\n<"
         << type_name.first << ">::" << type_name.second << "\n  "
         << std::setw(4) << config << std::endl;
  }
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
    std::cout << "Successfully Tested! Generated unique environment "
              << hash_fn(env_con.dump()) << std::endl;

    std::string env_name = env_con[0];
    auto env = life::make_environment(env_name);
    env.configure(env_con[1]);
    std::cout << "loaded experiment with environment from file \"" << env_exp_path << "\" ... \n";

	//auto res_pop = env.evaluate(pop);


    std::cout << "\nYay?? Ran succesfully?\n";

  } else {
    std::cout << "ded: unknown command line arguments. try -h\n";
    // auto e = life::make_entity("bit_brain");
    // auto b = e;
    // std::cout << (b < e);
  }
}
