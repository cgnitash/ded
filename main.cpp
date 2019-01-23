

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

  for (auto &group :
       {"parameters", "pre-tags", "post-tags", "input-tags", "output-tags"})
    if (config.find(group) == config.end()) {
      std::cout << "User publication error: user module<" << type_name.first
                << ">::'" << type_name.second << "' must publish '" << group
                << "' group\n";
      exit(1);
    }

  if (config.size() != 5) {
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

  for (auto &group : {"parameters", "input-tags", "output-tags"})
    if (config.find(group) == config.end()) {
      std::cout << "User publication error: user module<" << type_name.first
                << ">::'" << type_name.second << "' must publish '" << group
                << "' group\n";
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

auto signal_name_type(std::string s) {
  std::regex r{R"~~(^([-\w\d]+),([-\>\<\w\d,]+)$)~~"};
  std::smatch m;
  std::regex_match(s, m, r);
  return std::make_pair(m[1].str(), m[2].str());
}

bool tag_converts_to(bool in_order, std::string pub, std::string req,
                     life::configuration pop_config,
                     life::configuration env_config) {

  if (!in_order) {
    std::swap(pub, req);
    std::swap(pop_config, env_config);
  }

  // req matches any input
  if (req.empty())
    return true;

  std::regex pod{R"~~(^(double|long|bool)$)~~"};
  std::regex agg{R"~~(^A\<(double|long|bool)(,([-\w\d]+))?\>$)~~"};

  std::smatch m_req, m_pub;
  // pub and req are pods
  if (std::regex_match(pub, m_pub, pod) && std::regex_match(req, m_req, pod)) {
    return pub == req;
  }

  // exactly one of pub or req are pods
  if (std::regex_match(pub, m_pub, pod) || std::regex_match(req, m_req, pod)) {
    return false;
  }

  // both pub and req must be aggs
  if (std::regex_match(req, m_req, agg) && std::regex_match(pub, m_pub, agg)) {
    // pub and req type must match 
    if (m_pub[1].str() != m_req[1].str())
      return false;

    // req is an unconstrained agg
    if (m_req[2].str().empty())
        return true;

    // so req is constrained

    // pub is unconstrained
    if (m_pub[2].str().empty())
      return false;

	// so pub is constrained
    auto req_type = m_req[3].str();
    auto pub_type = m_pub[3].str();
    // req is constrained by number
    if (ranges::all_of(req_type, [](auto c) { return std::isdigit(c); })) {
      // pub is constrained by number
      if (ranges::all_of(pub_type, [](auto c) { return std::isdigit(c); }))
        return req_type == pub_type;
      // pub is constrained by parameter
      auto j_pub_val = env_config["parameters"][pub_type];
      auto pub_val = j_pub_val.get<int>();
      return pub_val == std::stoi(req_type);
    }
    // req is constrained by parameter
    auto j_req_val = pop_config["parameters"][req_type];
    auto req_val = j_req_val.get<int>();
    // pub is constrained by number
    if (ranges::all_of(pub_type, [](auto c) { return std::isdigit(c); }))
      return std::stoi(pub_type) == req_val;
    // pub is constrained by parameter
    auto j_pub_val = env_config["parameters"][pub_type];
    auto pub_val = j_pub_val.get<int>();
    return pub_val == req_val;
  }
  return false;
}

life::configuration
check_org_signal_tag_overrides(bool is_input, life::configuration::iterator it,
                               life::configuration::iterator rit,
                               life::configuration pop,
                               life::configuration env_config) {

  auto signal_category =
      is_input ? std::string{"input-tags"} : std::string{"output-tags"};
  auto name = it.key();
  auto nested_name = it.value()[0];
  auto reqs = pop[1][signal_category];
  auto nested_con = rit.value()[1];
  auto overs = it.value()[1][signal_category];
  auto published = env_config[signal_category];

 std::cout << "reqs" << reqs << std::endl;
 std::cout << "overs" << overs << std::endl;
 std::cout << "pop_con" << pop << std::endl;
 std::cout << "env_con" << env_config << std::endl;
 std::cout << "it" << it.value()[1] << std::endl;
 std::cout << "published" << published << std::endl;

  if (reqs.size() < published.size()) {
    std::cout << "error: Population<entity cannot handle all the "
              << signal_category
              << "-tags provided by environment::" << nested_name << "\n";
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
      std::cout << "error: \033[31m<environment>::" << nested_name
                << "\033[0m does not have an " << signal_category
                << "-signal named \033[31m\"" << key
                << "\"\033[0m to override\n";
      for (auto it : env_config[signal_category].items())
        if (life::config::match(key, it.key()))
          std::cout << "Did you mean \033[32m'" << it.key() << "'\033[0m?\n";
      std::exit(1);
    }
    auto find_req = ranges::find(req_split, std::string{value},
                                 &std::pair<std::string, std::string>::first);
    if (find_req == ranges::end(req_split)) {
      std::cout << "error: \033[31m<entity>::" << pop[0]
                << "\033[0m does not publish an " << signal_category
                << "-signal named \033[31m" << value
                << "\033[0m. This cannot be overridden\n";
      for (auto it : pop[1][signal_category].items())
        if (life::config::match(value, it.key()))
          std::cout << "Did you mean \033[32m'" << it.key() << "'\033[0m?\n";
      std::exit(1);
    }
    if (!tag_converts_to(is_input, find_pub->second, find_req->second, pop[1],
                         nested_con)) {
      std::cout << "error: " << find_req->first << " must be type "
                << find_req->second << " but " << find_pub->first
                << " has type " << find_pub->second << "'\n";
      std::exit(1);
    }
    // all good -- assign
    attempted_over[key] = find_req->first + "," + find_req->second;
    req_split.erase(find_req);
    pub_split.erase(find_pub);
  }

  // apply remaining tags
  for (auto &[pub_name, pub_type] : pub_split) {
    auto find_replacement = ranges::find_if(
        req_split, [p = pub_type, pop_config = pop[1], nested_con, is_input](auto r) {
          return tag_converts_to(is_input, p, r.second, pop_config, nested_con);
        });
    if (find_replacement == ranges::end(req_split)) {
      std::cout << "error: for published " << signal_category << "-signal "
                << nested_name << "'" << pub_name
                << "' - no Signals provided by <entity>::" << pop[0]
                << " are convertible to " << pub_type;
      std::exit(1);
    }
    attempted_over[pub_name] =
        find_replacement->first + "," + find_replacement->second;
    find_replacement = ranges::find_if(
        find_replacement + 1, ranges::end(req_split),
        [p = pub_type, pop_config = pop[1], nested_con, is_input](auto r) {
          return tag_converts_to(is_input, p, r.second, pop_config, nested_con);
        });
    if (find_replacement != ranges::end(req_split)) {
      std::cout << "error: for published " << signal_category << "-signal "
                << nested_name << "'" << pub_name
                << "' - multiple Signals provided by <entity>::" << pop[0]
                << " are "
                   "convertible to "
                << pub_type;
      std::exit(1);
    }
  }

  return attempted_over;
}

life::configuration check_tag_overrides(bool is_pre,
                                        life::configuration::iterator user_it,
                                        life::configuration::iterator real_it,
                                        life::configuration pop,
                                        life::configuration nested_con) {

  auto name = user_it.key();
  auto nested_name = user_it.value()[0];
  auto reqs = is_pre ? real_it.value()[2] : real_it.value()[3];
  auto tag_category =
      is_pre ? std::string{"pre-tags"} : std::string{"post-tags"};
  auto overs = user_it.value()[1][tag_category];
  auto published = nested_con[tag_category];

 std::cout << "reqs" << reqs << std::endl;
 std::cout << "overs" << overs << std::endl;
 std::cout << "pop_con" << pop << std::endl;
 std::cout << "env_con" << nested_con << std::endl;
 std::cout << "it" << user_it.value()[1] << std::endl;
 std::cout << "published" << published << std::endl;

  if (reqs.size() != published.size()) {
    std::cout << "error: environment::" << name
              << " cannot handle/provide all the tags supplied/needed by "
                 "environment::"
              << nested_name << "\n";
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

  // apply override tags
  for (auto &[key, value] : overs.items()) {
    auto find_pub = ranges::find(pub_split, key,
                                 &std::pair<std::string, std::string>::first);
    if (find_pub == ranges::end(pub_split)) {
      std::cout << "error: " << nested_name << " does not require a "
                << tag_category << " named '" << key << "'\n";
      std::exit(1);
    }
    auto find_req = ranges::find(req_split, std::string{value},
                                 &std::pair<std::string, std::string>::first);
    if (find_req == ranges::end(req_split)) {
      std::cout << "error: " << nested_name << " does not publish a "
                << tag_category << " named '" << value << "'\n";
      std::exit(1);
    }
    if (!tag_converts_to(is_pre, find_pub->second, find_req->second, pop[1],
                         nested_con)) {
      std::cout << "error: " << find_req->first << " must be type "
                << find_req->second << " but " << find_pub->first
                << " has type " << find_pub->second << "'\n";
      std::exit(1);
    }
    // all good -- assign
    attempted_over[key] = find_req->first + "," + find_req->second;
    req_split.erase(find_req);
    pub_split.erase(find_pub);
  }

  // apply remaining tags
  for (auto &[req_name, req_type] : req_split) {
    auto find_replacement = ranges::find_if(
        pub_split, [r = req_type, pop_config = pop[1], nested_con, is_pre](auto p) {
          return tag_converts_to(is_pre, p.second, r, pop_config, nested_con);
        });
    if (find_replacement == ranges::end(pub_split)) {
      std::cout << "error: in requirements of " << name
                << "\nno published  tags are convertible\n";
      std::exit(1);
    }
    attempted_over[find_replacement->first] = req_name + "," + req_type;
    find_replacement = ranges::find_if(
        find_replacement + 1, ranges::end(pub_split),
        [r = req_type, pop_config = pop[1], nested_con, is_pre](auto p) {
          return tag_converts_to(is_pre, p.second, r, pop_config, nested_con);
        });
    if (find_replacement != ranges::end(pub_split)) {
      std::cout << "error: ambiguity in requirements of " << name
                << "\nmultiple published  tags are convertible\n";
      std::exit(1);
    }
  }

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
                                            life::configuration user_env_config,
                                            life::configuration user_pop) {

  auto real_con = life::config::true_parameters(mip);

  for (auto user_it = user_env_config["parameters"].begin();
       user_it != user_env_config["parameters"].end(); user_it++) {
    auto real_it = real_con["parameters"].find(user_it.key());
    if (real_it == real_con["parameters"].end())
      life::config::config_mismatch_error(user_it.key(), mip);

    if (real_it->type_name() != user_it->type_name())
      life::config::type_mismatch_error(user_it.key(), real_it->type_name(),
                                        user_it->type_name(), mip);

    // non <environment> parameters are not checked
    if (user_it->type_name() != std::string{"array"} ||
        real_it.value()[0] != "null_environment") {
      real_it.value() = user_it.value();
    }
  }

  // check all <environment> parameters
  for (auto real_it = real_con["parameters"].begin();
       real_it != real_con["parameters"].end(); real_it++) {
    if (real_it->type_name() == std::string{"array"} &&
        real_it.value()[0] == "null_environment") {
      auto user_it = user_env_config["parameters"].find(real_it.key());
      if (user_it != user_env_config["parameters"].end()) {
        auto nested_env_config =
            life::config::true_parameters({"environment", user_it.value()[0]});
        auto pre_tags =
            check_tag_overrides(true, user_it, real_it, user_pop, nested_env_config);
        auto post_tags =
            check_tag_overrides(false, user_it, real_it, user_pop, nested_env_config);
        real_it.value() = life::configuration::array(
            {user_it.value()[0],
             true_environment_object({"environment", user_it.value()[0]},
                                     user_it.value()[1], user_pop)});
        auto org_in_tags = check_org_signal_tag_overrides(
            true, user_it, real_it, user_pop, nested_env_config);
        auto org_out_tags = check_org_signal_tag_overrides(
            false, user_it, real_it, user_pop, nested_env_config);
        real_it.value()[1]["pre-tags"] = pre_tags;
        real_it.value()[1]["post-tags"] = post_tags;
        real_it.value()[1]["input-tags"] = org_in_tags;
        real_it.value()[1]["output-tags"] = org_out_tags;
      } else {
        check_unmentioned_tag_overrides(mip, real_it.key(), real_it.value());
      }
    }
  }
  return real_con;
}

void pretty_show_entity(life::ModuleInstancePair mip, life::configuration con) {

  std::cout << "\033[31mentity::" << mip.second << "\033[0m\n";

  for (std::string group : {"parameters", "input-tags", "output-tags"}) {
    std::cout << "\033[33m" << group << "----\033[0m\n";
    for (auto &[key, value] : con[group].items())
      if (value.type_name() != std::string{"array"})
        std::cout << std::setw(26) << key << " : " << value << "\n";
    std::cout << "\033[33m" << std::string(group.length(), ' ')
              << "----\033[0m\n";
  }
}

void pretty_show_environment(life::ModuleInstancePair mip,
                             life::configuration con) {

  std::cout << "\033[31menvironment::" << mip.second << "\033[0m\n";

  for (std::string group :
       {"parameters", "pre-tags", "post-tags", "input-tags", "output-tags"}) {
    std::cout << "\033[33m" << group << "----\033[0m\n";
    for (auto &[key, value] : con[group].items())
      if (value.type_name() != std::string{"array"})
        std::cout << std::setw(26) << key << " : " << value << "\n";
    std::cout << "\033[33m" << std::string(group.length(), ' ')
              << "----\033[0m\n";
  }

  for (auto &[key, value] : con["parameters"].items())
    if (value.type_name() == std::string{"array"} &&
        value[0] == "null_environment") {
      std::cout << "\033[32mNested Environment ----\033[0m \033[31m" << key
                << "\033[0m\n";
      if (value[2] != nullptr)
        std::cout << "with pre-tag requirements \033[32m" << value[2]
                  << "\033[0m\n";
      if (value[3] != nullptr)
        std::cout << "with post-tag requirements \033[32m" << value[3]
                  << "\033[0m\n";
      std::cout << "\033[32m                   ----\033[0m\n";
    }
}

void pretty_show_population(life::ModuleInstancePair mip,
                            life::configuration con) {
  std::cout << "\033[31mpopulation::" << mip.second
            << "\033[0m\n\033[33mDefault Parameters ----\033[0m\n";
  for (auto &[key, value] : con["parameters"].items())
    std::cout << std::setw(26) << key << " : " << value << "\n";
  std::cout << "\033[33m               ----\033[0m\n";
}

void list_all_configs() {

  std::map<std::string, std::vector<std::string>> cons;
  for (auto &c : life::all_configs) 
    cons[c.first.first].push_back(c.first.second);
  for (auto &type : {"entity", "environment", "population"}) {
    std::cout << type << "\n";
    for (auto &name : cons[type])
      std::cout << "    " << name << "\n";
  }
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

std::vector<std::tuple<life::configuration, life::configuration, std::string>>
true_experiments(std::string file_name, std::hash<std::string> hash_fn) {

  std::vector<std::tuple<life::configuration, life::configuration, std::string>>
      exps;

  qst_parser q;
  for (auto &[pop_con_s, env_con_s, label] : q.parse_qst(file_name)) {

    // why this circumlocution?
    std::stringstream es, ps;
    ps << pop_con_s;
    es << env_con_s;
    life::configuration pop_con;
    life::configuration env_con;
    ps >> pop_con;
    es >> env_con;
    auto true_pop = life::configuration::array(
        {pop_con[0], true_any_object({"population", pop_con[0]}, pop_con[1])});
	
	// assume population is linear ***** 
    auto true_env = life::configuration::array(
        {env_con[0],
         true_environment_object({"environment", env_con[0]}, env_con[1],
                                 true_pop[1]["parameters"]["entity"])});

    exps.push_back(std::make_tuple(true_pop, true_env, label));
    auto exp_name = std::to_string(hash_fn(true_pop.dump())) + "_" +
                    std::to_string(hash_fn(true_env.dump()));
    std::cout << "Verified experiment " << exp_name << " with label \"" << label
              << "\"" << std::endl;
  }
  return exps;
}


long life::entity::entity_id_ = 0;

std::string life::global_path = "./";

std::map<life::ModuleInstancePair, life::configuration> life::all_configs;

int main(int argc, char **argv) {
  // TODO use an actual command-line library :P
  //

  life::generate_all_configs();
  // check all things that aren't being checked statically, in particular the
  // publications of components
  check_all_configs_correct();

  std::hash<std::string> hash_fn{};

  if (argc == 2 && std::string(argv[1]) == "-h") {
    std::cout << R"~~(
			  -s                     : saves configuration files 
              -rl <N> <file-name>    : 'runs' all experiments in this file-name with N replicates (locally)
              -rh <N> <file-name>    : 'runs' all experiments in this file-name with N replicates (msu hpc)
              -v <file-name>         : verify experiment in file-name
              -p <component-name>... : print publication for listed component names
              -pa 					 : lists all components currently loaded
              -f <N> <file-name>     : actually runs this experiment with REP N (should NOT be called manually)
				)~~";
  } else if (argc == 2 && std::string(argv[1]) == "-s") {
    std::cout << "saving configurations.cfg ... \n";
    save_configs();
  } else if (argc == 2 && std::string(argv[1]) == "-pa") {
    list_all_configs();
    std::cout << std::endl;
  } else if (argc > 2 && std::string(argv[1]) == "-p") {
    for (auto i{2}; i < argc; i++)
      show_config(std::string(argv[i]));
    std::cout << std::endl;
  } else if (argc == 3 && std::string(argv[1]) == "-v") {
    true_experiments(argv[2],hash_fn);
    std::cout << "\nVerified all experiments succesfully\n";

  } else if (argc == 4 && ((std::string(argv[1]) == "-rl") ||
                           (std::string(argv[1]) == "-rh"))) {
    std::string qst_path = argv[3];
    life::global_path = qst_path.substr(0, qst_path.find_last_of('/') + 1) + "data/";

    if (!std::experimental::filesystem::exists(life::global_path))
      std::experimental::filesystem::create_directory(life::global_path);

    std::vector<std::string> exps;
    for (auto &[pop, env, label] : true_experiments(qst_path, hash_fn)) {

      // label unused for now
      (void)label;

      auto exp_name = std::to_string(hash_fn(pop.dump())) + "_" +
                      std::to_string(hash_fn(env.dump()));
      auto exp_path = life::global_path + exp_name;
      if (std::experimental::filesystem::exists(exp_path)) {
        std::cout << "error: these experiments already exist\n";
        std::exit(1);
      }
      std::experimental::filesystem::create_directory(exp_path);

      exps.push_back(exp_name);
      std::ofstream pop_file(exp_path + "/true_pop.json");
      pop_file << pop.dump(4);

      std::ofstream env_file(exp_path + "/true_env.json");
      env_file << env.dump(4);
    }
    std::ofstream run_file("run.sh");
    if (std::string(argv[1]) == "-rl") {
      run_file << "for i in "
               << ranges::accumulate(
                      exps, std::string{},
                      [](auto ret, auto s) { return ret + s + " "; })
               << " ; do for r in "
               << ranges::accumulate(ranges::view::iota(0, std::stoi(argv[2])),
                                     std::string{},
                                     [](auto s, auto i) {
                                       return s + std::to_string(i) + " ";
                                     })
               << " ; do ./ded -f $r " << life::global_path
               << "$i ; done  ; done";
    } else { //  	if (std::string(argv[1] == "-rh")
      std::ofstream sb_file("run.sb");
      sb_file << R"~~(#!/bin/bash -login
#SBATCH --time=03:56:00
#SBATCH --mem=2GB
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
		)~~";
      sb_file << "\n#SBATCH --array=1-" << argv[2]
              << "\ncd ${SLURM_SUBMIT_DIR}\n./ded -f "
                 "${SLURM_ARRAY_TASK_ID} "
              << life::global_path << "$1\n";
      for (auto &e : exps)
        run_file << "\nsbatch run.sb " << e;
    }
    std::cout << "\nGenerated script run.sh succesfully\n";
  } else if (argc == 4 && std::string(argv[1]) == "-f") {
    auto exp_dir = std::string{argv[3]};
    if (!std::experimental::filesystem::exists(exp_dir)) {
      std::cout << "error: no directory " << exp_dir
                << " found. Please DON'T modify the data/ directory manually\n";
      std::exit(1);
    }

    life::configuration env_con, pop_con;
    std::ifstream pop_file(exp_dir + "/true_pop.json");
    pop_file >> pop_con;

    std::ifstream env_file(exp_dir + "/true_env.json");
    env_file >> env_con;

    life::global_path = exp_dir + "/REP_" + argv[2] + "/";
    std::experimental::filesystem::create_directory(life::global_path);
    std::srand(std::stoi(argv[2]));
    auto pop = life::make_population(pop_con);

    auto env = life::make_environment(env_con);

    env.evaluate(pop);

    std::cout << "\nExperiment " << exp_dir << "with rep:" << argv[2]
              << " run succesfully\n";

  } else {
    std::cout << "ded: unknown command line arguments. try -h\n";
  }
}
