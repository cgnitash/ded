

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


void check_environment_correct(life::ModuleInstancePair type_name,
                       life::configuration config) {
  if (config.find("parameters") == config.end()) {
    std::cout << "User publication error: user module<"
              << type_name.first << ">::'" << type_name.second
              << "' must publish 'parameters' group"
              << "\n";
    exit(1);
  }

  if (config.find("pre-tags") == config.end()) {
    std::cout << "User publication error: user module<"
              << type_name.first << ">::'" << type_name.second
              << "' must publish 'pre-tags' group"
              << "\n";
    exit(1);
  }

  if (config.find("post-tags") == config.end()) {
    std::cout << "User publication error: user module<"
              << type_name.first << ">::'" << type_name.second
              << "' must publish 'post-tag' group"
              << "\n";
    exit(1);
  }

  if (config.size() != 3) {
    std::cout << "User publication error: user module<"
              << type_name.first << ">::'" << type_name.second
              << "' must not publish unspecified groups"
              << "\n";
    exit(1);
  }
  return;
}
void check_population_correct(life::ModuleInstancePair type_name,
                       life::configuration config) {
  if (config.find("parameters") == config.end()) {
    std::cout << "User publication error: user module<"
              << type_name.first << ">::'" << type_name.second
              << "' must publish 'parameters' group"
              << "\n";
    exit(1);
  }

  if (config.size() != 1) {
    std::cout << "User publication error: user module<"
              << type_name.first << ">::'" << type_name.second
              << "' must not publish unspecified groups"
              << "\n";
    exit(1);
  }
  return;
}
void check_entity_correct(life::ModuleInstancePair type_name,
                       life::configuration config) {
  if (config.find("parameters") == config.end()) {
    std::cout << "User publication error: user module<"
              << type_name.first << ">::'" << type_name.second
              << "' must publish 'parameters' group"
              << "\n";
    exit(1);
  }

  if (config.size() != 1) {
    std::cout << "User publication error: user module<"
              << type_name.first << ">::'" << type_name.second
              << "' must not publish unspecified groups"
              << "\n";
    exit(1);
  }
  return;
}

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
  std::cout << "still in post " << attempted_over << "\n";

  return attempted_over;
}

/*
life::configuration true_population_object(life::ModuleInstancePair mip,
                                life::configuration con) {

  auto real_con = life::config::true_parameters(mip);

  life::configuration attempted_pre_tags;
  for (auto it = con["parameters"].begin(); it != con["parameters"].end();
       it++) {
    auto rit = real_con["parameters"].find(it.key());
    if (rit == real_con["parameters"].end())
      life::config::config_mismatch_error(it.key(), mip);

    if (rit->type_name() != it->type_name())
      life::config::type_mismatch_error(it.key(), rit->type_name(), it->type_name(), mip);

    if (it->type_name() == std::string{"array"}) {
      rit.value() = life::configuration::array(
          {it.value()[0], true_population_object(
                              {"population", it.value()[0]}, it.value()[1])});
    } else {
      rit.value() = it.value();
    }
  }
  return real_con;
}
*/
life::configuration true_any_object(life::ModuleInstancePair mip,
                                life::configuration con) {

  auto real_con = life::config::true_parameters(mip);

  for (auto it = con["parameters"].begin(); it != con["parameters"].end();
       it++) {
    auto rit = real_con["parameters"].find(it.key());
    if (rit == real_con["parameters"].end())
      life::config::config_mismatch_error(it.key(), mip);

    if (rit->type_name() != it->type_name())
      life::config::type_mismatch_error(it.key(), rit->type_name(),
                                        it->type_name(), mip);

    rit.value() =
        it->type_name() == std::string{"array"}
            ? life::configuration::array(
                  {it.value()[0],
                   true_any_object(
                       {std::string{rit.value()[0]}.substr(5), it.value()[0]},
                       it.value()[1])})
            : it.value();
  }
  return real_con;
}


life::configuration true_environment_object(life::ModuleInstancePair mip,
                                life::configuration con) {

  auto real_con = life::config::true_parameters(mip);

  for (auto it = con["parameters"].begin(); it != con["parameters"].end();
       it++) {
    auto rit = real_con["parameters"].find(it.key());
    if (rit == real_con["parameters"].end())
      life::config::config_mismatch_error(it.key(), mip);

    if (rit->type_name() != it->type_name())
      life::config::type_mismatch_error(it.key(), rit->type_name(), it->type_name(), mip);

    if (it->type_name() == std::string{"array"} &&
        rit.value()[0] == "null_environment") {
      auto nested_con =
          life::config::true_parameters({"environment", it.value()[0]});
      auto pre_tags = check_pre_tag_overrides(
          it.key(), it.value()[0], rit.value()[2], it.value()[1]["pre-tags"],
          nested_con["pre-tags"]);
      auto post_tags = check_post_tag_overrides(
          it.key(), it.value()[0], rit.value()[3], it.value()[1]["post-tags"],
          nested_con["post-tags"]);
      rit.value() = life::configuration::array(
          {it.value()[0], true_environment_object(
                              {"environment", it.value()[0]}, it.value()[1])});
	  rit.value()[1]["pre-tags"] = pre_tags;
	  rit.value()[1]["post-tags"] = post_tags;
    } else {
      rit.value() = it.value();
    }
  }
  return real_con;
}

life::configuration true_user_population(std::string file_name) {
  life::configuration con;
  std::ifstream ifs(file_name);
  if (!ifs.is_open()) {
    std::cout << "Error: population file \"" << file_name
              << "\" does not exist\n";
    std::exit(1);
  }
  ifs >> con;
  return life::configuration::array(
      {con[0], life::config::true_object({"population", con[0]}, con[1])});
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
  for (auto &[type_name, config] : life::all_configs) 
    file << "\n<"
         << type_name.first << ">::" << type_name.second << "\n  "
         << std::setw(4) << config << std::endl;
  
}

void check_all_configs_correct() {

  for (auto &[type_name, config] : life::all_configs) {
    if (type_name.first == "environment") 
		check_environment_correct(type_name,config);
    if (type_name.first == "entity") 
		check_entity_correct(type_name,config);
    if (type_name.first == "population") 
		check_population_correct(type_name,config);
  }
}

auto missing_module_instance_error(life::ModuleInstancePair mip) {
  auto &true_mod = mip.first;
  auto &attempted_inst = mip.second;
  std::cout << "Error: Non-existent <Module>::Instance -- \033[31m<" << true_mod
            << ">::" << attempted_inst << "\033[0m\n";
  for (auto &type_name_config_pair : life::all_configs) {
    auto &[mod, inst] = type_name_config_pair.first;
    if (mod == true_mod && life::config::match(attempted_inst,inst))
      std::cout << "Did you mean \033[32m'" << inst << "'\033[0m?\n";
  }
  std::exit(1);
}


life::configuration check_config_exists(life::ModuleInstancePair mip) {
  auto real_con_it = life::all_configs.find(mip);
  if (life::all_configs.end() == real_con_it)
	 missing_module_instance_error(mip); 
  return real_con_it->second;
}

auto parse_qst(std::string file_name) {

  std::regex comments{R"~~(#.*$)~~"};
  //std::regex spaces{R"~~(\s+)~~"};
  std::regex close_brace{R"~~(^\s*}\s*$)~~"};
  std::regex parameter{R"~~(^\s*p\s*([-\w\d]+)\s*=\s*([-\w\d]+)\s*$)~~"};
  //std::regex param{R"~~(^\s*p\s*([-\w\d]+)\s*=\s*([-\w\d]+)\s*$)~~"};
  // std::regex nested_parameter{
  //  R"~~(^\s*p\s*:\s*([-\w]+)\s*=\s*\$([\w\d])\s*(\{)?\s*$)~~"};
  //std::regex object{R"~~(\$([-\w\d]+))~~"};
  std::regex new_variable{
      R"~~(^\s*([\w\d]+)\s*=\s*\$([-\w\d]+)\s*(\{)?\s*$)~~"};
  std::regex nested_parameter{
      R"~~(^\s*p\s+([-\w\d]+)\s*=\s*\$([-\w\d]+)\s*(\{)?\s*$)~~"};

  std::regex spurious_commas{R"~~(,(]|}))~~"};

  std::ifstream ifs(file_name);
  if (!ifs.is_open()) {
    std::cout << "Error: qst file \"" << file_name << "\" does not exist\n";
    std::exit(1);
  }

  std::map<std::string, std::string> all_variables;
//  std::vector<std::pair<std::string, std::string>> command_stack;
  std::string line;
  std::string current_variable_name;
  std::smatch m;
  auto brace_count{0};
  for (auto line_num{1}; std::getline(ifs, line); line_num++) {

    line = std::regex_replace(line, comments, "");

    if (line.empty()) 
      continue;

    if (std::regex_match(line, m, new_variable)) {
      brace_count += !m[3].str().empty();
      if (!current_variable_name.empty()) {
        std::cout << "qst<syntax>error: new user variable cannot be nested "
                     "within "
                     "other components! line "
                  << line_num << "\n";
        std::exit(1);
          }
                  current_variable_name = m[1].str();
	  all_variables[current_variable_name] = "[\"" + m[2].str() + "\",{\"parameters\":{"; 
      continue;
    }

    if (std::regex_match(line, m, nested_parameter)) {
      brace_count += !m[3].str().empty();
      if (current_variable_name.empty()) {
        std::cout << "qst<syntax>error: new user variable cannot be nested "
                     "within "
                     "other components! line "
                  << line_num << "\n";
        std::exit(1);
          }
          all_variables[current_variable_name] += "\"" + m[1].str() + "\":[\"" +
                                                  m[2].str() +
                                                  "\",{\"parameters\":{";
          continue;
    }
    if (std::regex_match(line, m, parameter)) {
      std::cout << "matched parameter regex with - " << line << std::endl;
      all_variables[current_variable_name] +=
          std::regex_replace(line, parameter, "\"$1\":$2,");
      continue;
	}

    if (std::regex_match(line, m, close_brace)) {
      std::cout << "matched closed brace regex with - " << line << std::endl;
      if (current_variable_name.empty()) {
        std::cout << "qst<syntax>error: dangling closing brace! line "
                  << line_num << "\n";
        std::exit(1);
      }
        all_variables[current_variable_name] += "}}],";
      brace_count--;
      if (!brace_count) {
        all_variables[current_variable_name] += "}}]";
        current_variable_name.clear();
      } 
      continue;
    }

    std::cout << "qst<syntax>error: unable to parse! line " << line_num << "\n"
              << line << "\n";
    std::exit(1);

  }

  if (!current_variable_name.empty()) {
    std::cout << "qst<syntax>error: braces need to be added\n";
    std::exit(1);
  }
  /*
            auto env_con =
                std::regex_replace(all_variables["E"], close_brace, "}]");
            env_con =
                std::regex_replace(env_con, parameter,
     "\"parameters\":{\"$1\":"); env_con = std::regex_replace(env_con, object,
     "[\"$1\","); env_con = std::regex_replace(env_con, spaces, "");

            auto pop_con =
                std::regex_replace(all_variables["P"], close_brace, "}]");
            pop_con = std::regex_replace(pop_con, parameter,
                                         "\"parameters\":{\"$1\"},");
            pop_con = std::regex_replace(pop_con, object, "[\"$1\",");
            pop_con = std::regex_replace(pop_con, spaces, "");
  */
  auto env_con = std::regex_replace(all_variables["E"], spurious_commas, "$1");
  auto pop_con = std::regex_replace(all_variables["P"], spurious_commas, "$1");
  std::cout << env_con << "\n" << pop_con << std::endl;
  std::stringstream es, ps;
  es << env_con;
  ps << pop_con;

  life::configuration env, pop;
  es >> env;
  ps >> pop;
  // life::configuration env_pop;
   
  std::cout << env.dump(4) << "\n" << pop.dump(4) << std::endl;
  return std::make_pair(pop, env);
  /*
  for(auto &[k,v] : all_variables)
          std::cout << k << "\n" << v << "\n";
*/
  // auto pop_con = true_user_population(pop_exp_path);
  // auto env_con = true_user_environment(env_exp_path);
  //
}

long life::entity::entity_id_ = 0;

std::string life::global_path = "./";

std::map<life::ModuleInstancePair, life::configuration> life::all_configs;

int main(int argc, char **argv) {
  // TODO use an actual command-line library :P
  //

  life::generate_all_configs();
  check_all_configs_correct();

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

	auto res_pop = env.evaluate(pop);
	for (auto o : res_pop.get_as_vector())
          std::cout << o.get_id() << " ";
	
    std::cout << "\nYay?? Ran succesfully?\n";

  } else if (argc == 3 && std::string(argv[1]) == "-b") {
    std::string qst_path = argv[2];
    life::global_path = qst_path.substr(0, qst_path.find_last_of('/') + 1);

    std::hash<std::string> hash_fn;

    auto [pop_con, env_con] = parse_qst(qst_path);

    auto true_pop = life::configuration::array(
        {pop_con[0],
         true_any_object({"population", pop_con[0]}, pop_con[1])});
    auto true_env = life::configuration::array(
        {env_con[0],
         true_environment_object({"environment", env_con[0]}, env_con[1])});

    std::cout << std::setw(4) << true_pop << std::endl;
    std::cout << "\nNot yet Tested! Generated unique population "
              << hash_fn(true_pop.dump()) << std::endl;

    std::cout << std::setw(4) << true_env << std::endl;
    std::cout << "\nNot yet Tested! Generated unique environment "
              << hash_fn(true_env.dump()) << std::endl;

    std::string pop_name = true_pop[0];
    auto pop = life::make_population(pop_name);
    pop.configure(true_pop[1]);

    std::string env_name = true_env[0];
    auto env = life::make_environment(env_name);
    env.configure(true_env[1]);

    auto res_pop = env.evaluate(pop);
    for (auto o : res_pop.get_as_vector())
      std::cout << o.get_id() << " ";
	
    std::cout << "\nYay?? Ran succesfully?\n";

  } else {
    std::cout << "ded: unknown command line arguments. try -h\n";
    // auto e = life::make_entity("bit_brain");
    // auto b = e;
    // std::cout << (b < e);
  }
}
