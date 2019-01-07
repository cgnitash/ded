

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
    std::cout << "User publication error: user module<" << type_name.first
              << ">::'" << type_name.second
              << "' must publish 'parameters' group"
              << "\n";
    exit(1);
  }

  if (config.find("pre-tags") == config.end()) {
    std::cout << "User publication error: user module<" << type_name.first
              << ">::'" << type_name.second << "' must publish 'pre-tags' group"
              << "\n";
    exit(1);
  }

  if (config.find("post-tags") == config.end()) {
    std::cout << "User publication error: user module<" << type_name.first
              << ">::'" << type_name.second << "' must publish 'post-tag' group"
              << "\n";
    exit(1);
  }

  if (config.size() != 3) {
    std::cout << "User publication error: user module<" << type_name.first
              << ">::'" << type_name.second
              << "' must not publish unspecified groups"
              << "\n";
    exit(1);
  }
  return;
}
void check_population_correct(life::ModuleInstancePair type_name,
                              life::configuration config) {
  if (config.find("parameters") == config.end()) {
    std::cout << "User publication error: user module<" << type_name.first
              << ">::'" << type_name.second
              << "' must publish 'parameters' group"
              << "\n";
    exit(1);
  }

  if (config.size() != 1) {
    std::cout << "User publication error: user module<" << type_name.first
              << ">::'" << type_name.second
              << "' must not publish unspecified groups"
              << "\n";
    exit(1);
  }
  return;
}
void check_entity_correct(life::ModuleInstancePair type_name,
                          life::configuration config) {
  if (config.find("parameters") == config.end()) {
    std::cout << "User publication error: user module<" << type_name.first
              << ">::'" << type_name.second
              << "' must publish 'parameters' group"
              << "\n";
    exit(1);
  }

  if (config.size() != 1) {
    std::cout << "User publication error: user module<" << type_name.first
              << ">::'" << type_name.second
              << "' must not publish unspecified groups"
              << "\n";
    exit(1);
  }
  return;
}

auto signal_name_type(std::string s) {
  std::regex r{R"~~(^([-\w\d]+),([-\w\d,]+)$)~~"};
  std::smatch m;
  std::regex_match(s, m, r);
  return std::make_pair(m[1].str(), m[2].str());
}

life::configuration check_pre_tag_overrides(std::string name,
                                            std::string nested_name,
                                            life::configuration reqs,
                                            life::configuration overs,
                                            life::configuration published) {

  std::cout << "in pre of " << name << " :required " << reqs << " "
            << reqs.size() << "\n and :published " << published << " "
            << published.size() << "\n";
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

  std::vector<std::pair<std::string, std::string>> req_split;
  for (auto &r : reqs)
    req_split.push_back(signal_name_type(r));

  // try ::as_lvalue  ??
  std::vector<std::pair<std::string, std::string>> pub_split;
  for (auto &it : published.items())
    pub_split.push_back(signal_name_type(it.value()));

  life::configuration attempted_over = published;

  for (auto &[key, value] : overs.items()) {
    auto find_pub = ranges::find(pub_split, key,
                                 &std::pair<std::string, std::string>::first);
    if (find_pub == ranges::end(pub_split)) {
      std::cout << "error: " << nested_name
                << " does not require a pre-tag named '" << key << "'\n";
      std::exit(1);
    }
    auto find_req = ranges::find(req_split, std::string{value},
                                 &std::pair<std::string, std::string>::first);
    if (find_req == ranges::end(req_split)) {
      std::cout << "error: " << nested_name
                << " does not publish a pre-tag named '" << value << "'\n";
      std::exit(1);
    }
    // this should be more involved
    if (find_req->second != find_pub->second) {
      std::cout << "error: " << find_req->first << " must be type "
                << find_req->second << " but " << find_pub->first
                << " has type " << find_pub->second << "'\n";
      std::exit(1);
    }
    // all good -- assign
    attempted_over[key] = find_req->first + "," + find_req->second;
    req_split.erase(ranges::remove_if(req_split,
                                      [v = value](auto r) {
                                        return r.first == std::string{v};
                                      }),
                    ranges::end(req_split));
    pub_split.erase(ranges::remove_if(
                        pub_split, [k = key](auto p) { return p.first == k; }),
                    ranges::end(pub_split));
  }

  std::map<std::string, int> types;
  for (auto &r : req_split)
    types[r.second]++;
  if (ranges::any_of(types, [](auto t) { return t.second > 1; })) {
    std::cout << "error: ambiguity resulting from multiple tags with same type "
                 "in requirements of "
              << name << "\n";
    std::exit(1);
  }

  for (auto &r : req_split) {
    auto find_replacement = ranges::find(
        pub_split, r.second, &std::pair<std::string, std::string>::second);
    if (find_replacement == ranges::end(pub_split)) {
      std::cout << "error: this should NEVER happen" << std::endl;
      std::exit(1);
    }
    attempted_over[find_replacement->first] = r.first + "," + r.second;
  }

  std::cout << "still in pre " << attempted_over << "\n";

  return attempted_over;
  /*
  auto attempted_over = published;
  attempted_over.begin().value() = reqs[0];
  if (!overs.empty() && attempted_over != overs) {
    std::cout << "error: explicit overrides of pre-tags " << overs
              << " is invalid.\nNote that an automatic application of "
              << attempted_over << " is available\n";
    exit(1);
  }
  */
}

life::configuration check_post_tag_overrides(std::string name,
                                             std::string nested_name,
                                             life::configuration reqs,
                                             life::configuration overs,
                                             life::configuration published) {

  std::cout << "in post of " << name << " :required " << reqs << " "
            << reqs.size() << "\n and :published " << published << " "
            << published.size() << "\n";
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

  std::vector<std::pair<std::string, std::string>> req_split;
  for (auto &r : reqs)
    req_split.push_back(signal_name_type(r));

  // try ::as_lvalue  ??
  std::vector<std::pair<std::string, std::string>> pub_split;
  for (auto &it : published.items())
    pub_split.push_back(signal_name_type(it.value()));

  life::configuration attempted_over = published;

  for (auto &[key, value] : overs.items()) {
    auto find_pub = ranges::find(pub_split, key,
                                 &std::pair<std::string, std::string>::first);
    if (find_pub == ranges::end(pub_split)) {
      std::cout << "error: " << nested_name
                << " does not publish a post-tag named '" << key << "'\n";
      std::exit(1);
    }
    auto find_req = ranges::find(req_split, std::string{value},
                                 &std::pair<std::string, std::string>::first);
    if (find_req == ranges::end(req_split)) {
      std::cout << "error: " << nested_name
                << " does not require a post-tag named '" << value << "'\n";
      std::exit(1);
    }
    // this should be more involved
    if (find_req->second != find_pub->second) {
      std::cout << "error: " << find_req->first << " must be type "
                << find_req->second << " but " << find_pub->first
                << " has type " << find_pub->second << "'\n";
      std::exit(1);
    }
    // all good -- assign
    attempted_over[key] = find_req->first + "," + find_req->second;
    req_split.erase(ranges::remove_if(req_split,
                                      [v = value](auto r) {
                                        return r.first == std::string{v};
                                      }),
                    ranges::end(req_split));
    pub_split.erase(ranges::remove_if(
                        pub_split, [k = key](auto p) { return p.first == k; }),
                    ranges::end(pub_split));
  }

  std::map<std::string, int> types;
  for (auto &r : req_split)
    types[r.second]++;
  if (ranges::any_of(types, [](auto t) { return t.second > 1; })) {
    std::cout << "error: ambiguity resulting from multiple tags with same type "
                 "in requirements of "
              << name << "\n";
    std::exit(1);
  }

  for (auto &r : req_split) {
    auto find_replacement = ranges::find(
        pub_split, r.second, &std::pair<std::string, std::string>::second);
    if (find_replacement == ranges::end(pub_split)) {
      std::cout << "error: this should NEVER happen" << std::endl;
      std::exit(1);
    }
    attempted_over[find_replacement->first] = r.first + "," + r.second;
  }
  std::cout << "still in post " << attempted_over << "\n";
  return attempted_over;
}

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

void check_unmentioned_tag_overrides(life::ModuleInstancePair mip,
                                     life::configuration key,
                                     life::configuration value) {
  if (value[2] != nullptr) {
    std::cout << "error: environment" << value[0]
              << " does not handle the pre-tags that " << mip.second
              << "::" << key << " needs to handle\n";
    std::exit(1);
  }
  if (value[3] != nullptr) {
    std::cout << "error: environment" << value[0]
              << " does not provide the post-tags that " << mip.second
              << "::" << key << " needs to provide\n";
    std::exit(1);
  }
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
      life::config::type_mismatch_error(it.key(), rit->type_name(),
                                        it->type_name(), mip);

    if (it->type_name() != std::string{"array"} ||
        rit.value()[0] != "null_environment") {
      rit.value() = it.value();
    }
  }

  for (auto rit = real_con["parameters"].begin();
       rit != real_con["parameters"].end(); rit++) {
    if (rit->type_name() == std::string{"array"} &&
        rit.value()[0] == "null_environment") {
      auto it = con["parameters"].find(rit.key());
      if (it != con["parameters"].end()) {
        auto nested_con =
            life::config::true_parameters({"environment", it.value()[0]});
        auto pre_tags = check_pre_tag_overrides(
            it.key(), it.value()[0], rit.value()[2], it.value()[1]["pre-tags"],
            nested_con["pre-tags"]);
        auto post_tags = check_post_tag_overrides(
            it.key(), it.value()[0], rit.value()[3], it.value()[1]["post-tags"],
            nested_con["post-tags"]);
        rit.value() = life::configuration::array(
            {it.value()[0],
             true_environment_object({"environment", it.value()[0]},
                                     it.value()[1])});
        rit.value()[1]["pre-tags"] = pre_tags;
        rit.value()[1]["post-tags"] = post_tags;
      } else {
        check_unmentioned_tag_overrides(mip, rit.key(), rit.value());
      }
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

void pretty_show_entity(life::ModuleInstancePair mip, life::configuration con) {
  std::cout << mip.second << " is an entity\nDefault Parameters\n";
  for (auto &[key, value] : con["parameters"].items())
    std::cout << std::setw(16) << key << " : " << value << "\n";
}

void pretty_show_environment(life::ModuleInstancePair mip,
                             life::configuration con) {
  std::cout << mip.second << " is an environment\nDefault Parameters\n";
  for (auto &[key, value] : con["parameters"].items())
    if (value.type_name() != std::string{"array"})
      std::cout << std::setw(16) << key << " : " << value << "\n";
  for (auto &[key, value] : con["parameters"].items())
    if (value.type_name() == std::string{"array"} &&
        value[0] == "null_environment") {
      std::cout << "Nested Environment" << std::setw(10) << key << " : ";
      if (value[2] != nullptr)
        std::cout << "which has pre-tag requirements " << value[2] << "\n";
      if (value[3] != nullptr)
        std::cout << "which has post-tag requirements " << value[3] << "\n";
    }
  std::cout << "Pre-Tags\n";
  for (auto &[key, value] : con["pre-tags"].items())
    std::cout << std::setw(16) << key << " : " << value << "\n";
  std::cout << "Post-Tags\n";
  for (auto &[key, value] : con["post-tags"].items())
    std::cout << std::setw(16) << key << " : " << value << "\n";
}

void pretty_show_population(life::ModuleInstancePair mip,
                            life::configuration con) {
  std::cout << mip.second << " is a population\nDefault Parameters\n";
  for (auto &[key, value] : con["parameters"].items())
    std::cout << std::setw(16) << key << " : " << value << "\n";
}

void show_config(std::string name) {

  auto found = false;
  for (auto &[type_name, config] : life::all_configs) {
    if (type_name.second == name) {
      found = true;
      if (type_name.first == "environment")
        pretty_show_environment(type_name, config);
      if (type_name.first == "population")
        pretty_show_population(type_name, config);
      if (type_name.first == "entity")
        pretty_show_entity(type_name, config);
    }
  }
  if (!found) {
    std::cout << "component " << name << " not found\n";
    for (auto &type_name_config : life::all_configs)
      if (life::config::match(name, type_name_config.first.second))
        std::cout << "Did you mean \033[32m<" << type_name_config.first.first
                  << ">::" << type_name_config.first.second << "\033[0m?\n";
  }
}

void save_configs() {

  std::ofstream file("configurations.cfg");
  for (auto &[type_name, config] : life::all_configs)
    file << "\n<" << type_name.first << ">::" << type_name.second << "\n  "
         << std::setw(4) << config << std::endl;
}

void check_all_configs_correct() {

  for (auto &[type_name, config] : life::all_configs) {
    if (type_name.first == "environment")
      check_environment_correct(type_name, config);
    if (type_name.first == "entity")
      check_entity_correct(type_name, config);
    if (type_name.first == "population")
      check_population_correct(type_name, config);
  }
}

auto missing_module_instance_error(life::ModuleInstancePair mip) {
  auto &true_mod = mip.first;
  auto &attempted_inst = mip.second;
  std::cout << "Error: Non-existent <Module>::Instance -- \033[31m<" << true_mod
            << ">::" << attempted_inst << "\033[0m\n";
  for (auto &type_name_config_pair : life::all_configs) {
    auto &[mod, inst] = type_name_config_pair.first;
    if (mod == true_mod && life::config::match(attempted_inst, inst))
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
  std::regex close_brace{R"~~(^\s*}\s*$)~~"};
  std::regex spurious_commas{R"~~(,(]|}))~~"};
  std::regex new_variable{
      R"~~(^\s*([\w\d]+)\s*=\s*\$([-\w\d]+)\s*(\{)?\s*$)~~"};
  std::regex nested_parameter{
      R"~~(^\s*p\s+([-\w\d]+)\s*=\s*\$([-\w\d]+)\s*(\{)?\s*$)~~"};
  std::regex parameter{R"~~(^\s*p\s*([-\w\d]+)\s*=\s*([\.\-\w\d]+)\s*$)~~"};
  std::regex pre_tag{R"~~(^\s*tre\s*([-\w\d]+)\s*=\s*([-\w\d]+)\s*$)~~"};
  std::regex post_tag{R"~~(^\s*tos\s*([-\w\d]+)\s*=\s*([-\w\d]+)\s*$)~~"};

  std::ifstream ifs(file_name);
  if (!ifs.is_open()) {
    std::cout << "Error: qst file \"" << file_name << "\" does not exist\n";
    std::exit(1);
  }

  std::map<std::string, std::string> all_variables;
  struct component_spec {
    std::string variable_or_comp_name, comp, params, pres, posts;
  };
  std::vector<component_spec> component_stack;
  std::string line;
  std::smatch m;
  for (auto line_num{1}; std::getline(ifs, line); line_num++) {

    line = std::regex_replace(line, comments, "");

    if (line.empty())
      continue;

    if (std::regex_match(line, m, new_variable)) {
      if (!component_stack.empty()) {
        std::cout << "qst<syntax>error: new user variable cannot be nested "
                     "within "
                     "other components! line "
                  << line_num << "\n";
        std::exit(1);
      }
      if (m[3].str().empty()) {
        all_variables[m[1].str()] = "[\"" + m[2].str() +
                                    "\",{\"parameters\":null,\"pre-tags\":"
                                    "null,\"post-tags\":null}]";
      } else {
        component_stack.push_back({m[1].str(), m[2].str(), "", "", ""});
      }
      continue;
    }

    if (std::regex_match(line, m, nested_parameter)) {
      if (component_stack.empty()) {
        std::cout << "qst<syntax>error: new user variable cannot be nested "
                     "within "
                     "other components! line "
                  << line_num << "\n";
        std::exit(1);
      }
      if (m[3].str().empty()) {
        component_stack.back().params += "\"" + m[1].str() + "\":[\"" +
                                         m[2].str() +
                                         "\",{\"parameters\":null,\"pre-tags\":"
                                         "null,\"post-tags\":null}],";
      } else {
        component_stack.push_back({m[1].str(), m[2].str(), "", "", ""});
      }
      continue;
    }

    if (std::regex_match(line, m, parameter)) {
      std::cout << "matched parameter regex with - " << line << std::endl;
      component_stack.back().params +=
          "\"" + m[1].str() + "\":" + m[2].str() + ",";
      continue;
    }

    if (std::regex_match(line, m, pre_tag)) {
      std::cout << "matched pre-tag regex with - " << line << std::endl;
      component_stack.back().pres +=
          "\"" + m[1].str() + "\":\"" + m[2].str() + "\",";
      continue;
    }

    if (std::regex_match(line, m, post_tag)) {
      std::cout << "matched post-tag regex with - " << line << std::endl;
      component_stack.back().posts +=
          "\"" + m[1].str() + "\":\"" + m[2].str() + "\",";
      continue;
    }

    if (std::regex_match(line, m, close_brace)) {
      std::cout << "matched closed brace regex with - " << line << std::endl;
      if (component_stack.empty()) {
        std::cout << "qst<syntax>error: dangling closing brace! line "
                  << line_num << "\n";
        std::exit(1);
      }
      auto current = component_stack.back();
      component_stack.pop_back();
      if (component_stack.empty()) {
        all_variables[current.variable_or_comp_name] =
            "[\"" + current.comp + "\",{\"parameters\":{" + current.params +
            "},\"pre-tags\":{" + current.pres + "},\"post-tags\":{" +
            current.posts + "}}]";
      } else {
        component_stack.back().params +=
            "\"" + current.variable_or_comp_name + "\":[\"" + current.comp +
            "\",{\"parameters\":{" + current.params + "},\"pre-tags\":{" +
            current.pres + "},\"post-tags\":{" + current.posts + "}}],";
      }
      continue;
    }

    std::cout << "qst<syntax>error: unable to parse! line " << line_num << "\n"
              << line << "\n";
    std::exit(1);
  }

  if (!component_stack.empty()) {
    std::cout << "qst<syntax>error: braces need to be added\n";
    std::exit(1);
  }

  auto env_con = std::regex_replace(all_variables["E"], spurious_commas, "$1");
  auto pop_con = std::regex_replace(all_variables["P"], spurious_commas, "$1");
  std::cout << env_con << "\n" << pop_con << std::endl;
  std::stringstream es, ps;
  es << env_con;
  ps << pop_con;

  life::configuration env, pop;
  es >> env;
  ps >> pop;

  std::cout << env.dump(4) << "\n" << pop.dump(4) << std::endl;
  return std::make_pair(pop, env);
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
  } else if (argc == 3 && std::string(argv[1]) == "-p") {
    show_config(std::string(argv[2]));
  }
  /* else if (argc == 3 && std::string(argv[1]) == "-g") {
    std::string exp_path = argv[2];
    life::global_path = exp_path.substr(0, exp_path.find_last_of('/') + 1);
    auto gen = life::make_experiment({"generator"});
    std::cout << "generating sub-experiments from file \"" << exp_path << "\"
  ... \n"; life::configuration con; con["file"] = exp_path; gen.configure(con);
    gen.run();
    std::cout << "\nSuccessfully Generated!\n";

  } else if (argc == 4 && std::string(argv[1]) == "-t") {
    std::string pop_exp_path = argv[2];
    life::global_path = pop_exp_path.substr(0, pop_exp_path.find_last_of('/') +
  1); std::string env_exp_path = argv[3]; std::string env_path =
  env_exp_path.substr(0, env_exp_path.find_last_of('/') + 1); if( env_path !=
  life::global_path) { std::cout << "error: both pop and env must come from same
  dir\n"; std::exit(1);
        }
    std::hash<std::string> hash_fn;

    auto pop_con = true_user_population(pop_exp_path);
    std::cout << std::setw(4) << pop_con << std::endl;
    std::cout << "\nNot yet Tested! Generated unique population "
              << hash_fn(pop_con.dump()) << std::endl;

    std::string pop_name = pop_con[0];
    auto pop = life::make_population(pop_name);
    pop.configure(pop_con[1]);
    std::cout << "loaded experiment with population from file \"" <<
  pop_exp_path << "\" ... \n";

    auto env_con = true_user_environment(env_exp_path);
    std::cout << std::setw(4) << env_con << std::endl;
    std::cout << "Successfully Tested! Generated unique environment "
              << hash_fn(env_con.dump()) << std::endl;

    std::string env_name = env_con[0];
    auto env = life::make_environment(env_name);
    env.configure(env_con[1]);
    std::cout << "loaded experiment with environment from file \"" <<
  env_exp_path << "\" ... \n";

        auto res_pop = env.evaluate(pop);
        for (auto o : res_pop.get_as_vector())
          std::cout << o.get_id() << " ";

    std::cout << "\nYay?? Ran succesfully?\n";

  }*/
  else if (argc == 3 && std::string(argv[1]) == "-b") {
    std::string qst_path = argv[2];
    life::global_path = qst_path.substr(0, qst_path.find_last_of('/') + 1);

    std::hash<std::string> hash_fn;

    auto [pop_con, env_con] = parse_qst(qst_path);

    auto true_pop = life::configuration::array(
        {pop_con[0], true_any_object({"population", pop_con[0]}, pop_con[1])});
    auto true_env = life::configuration::array(
        {env_con[0],
         true_environment_object({"environment", env_con[0]}, env_con[1])});

    std::cout << std::setw(4) << true_pop << std::endl;
    std::cout << "\nNot yet Tested! Generated unique population "
              << hash_fn(true_pop.dump()) << std::endl;

    std::cout << std::setw(4) << true_env << std::endl;
    std::cout << "\nNot yet Tested! Generated unique environment "
              << hash_fn(true_env.dump()) << std::endl;

    auto pop = life::make_population(true_pop);

    auto env = life::make_environment(true_env);

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
