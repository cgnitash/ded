

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
       {"parameters", "pre-tags", "post-tags", "org-inputs", "org-outputs"})
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

bool tag_converts_to(std::string pub, std::string req,
                     life::configuration pop_config,
                     life::configuration env_config) {
  if (pub == req)
    return true;
  std::regex pod{R"~~(^(double|long|bool)$)~~"};
  std::regex agg{R"~~(^A\<(double|long|bool)(,([-\w\d]+))?\>$)~~"};

  std::smatch m_req, m_pub;
  // pub and req are pods
  if (std::regex_match(pub, m_pub, pod) && std::regex_match(req, m_req, pod)) {
    if (m_pub[1].str() == "long" && m_req[1].str() == "double")
      return true;
    return false;
  }

  // exactly one of pub or req are pods
  if (std::regex_match(pub, m_pub, pod) || std::regex_match(req, m_req, pod)) {
    return false;
  }

  // both pub and req must be aggs
  if (std::regex_match(req, m_req, agg) && std::regex_match(pub, m_pub, agg)) {
    // req is an unconstrained agg
    if (m_req[2].str().empty())
      // pub is primitive-convertible to req
      if (m_pub[1].str() == "long" && m_req[1].str() == "double")
        return true;

    // req is a constrained agg

    // pub and req type match or are primitive-convertible
    if ((m_pub[1].str() != m_req[1].str()) &&
        (m_pub[1].str() != "long" || m_req[1].str() != "double"))
      return false;

    // pub is unconstrained
    if (m_pub[2].str().empty())
      return false;

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
                               life::configuration nested_con,
                               life::configuration pop_config,
                               life::configuration env_config) {

  auto signal_category =
      is_input ? std::string{"input"} : std::string{"output"};
  auto name = it.key();
  auto nested_name = it.value()[0];
  auto reqs = pop_config[signal_category + "-tags"];
  auto overs = it.value()[1]["org-" + signal_category + "s"];
  auto published = env_config["org-" + signal_category + "s"];

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
      std::cout << "error: " << nested_name << " does not have an "
                << signal_category << "-signal named '" << key
                << "' to override\n";
      std::exit(1);
    }
    auto find_req = ranges::find(req_split, std::string{value},
                                 &std::pair<std::string, std::string>::first);
    if (find_req == ranges::end(req_split)) {
      std::cout << "error: Population<entity> does not respond to an "
                << signal_category << "-signal named '" << value
                << "'. This cannot be overridden\n";
      std::exit(1);
    }
    if (!tag_converts_to(find_pub->second, find_req->second, pop_config,
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
        req_split, [p = pub_type, pop_config, nested_con](auto r) {
          return tag_converts_to(p, r.second, pop_config, nested_con);
        });
    if (find_replacement == ranges::end(req_split)) {
      std::cout
          << "error: for published " << signal_category << "-signal "
          << nested_name << "'" << pub_name
          << "' - no Signals provided by Population<entity> are convertible to "
          << pub_type;
      std::exit(1);
    }
    attempted_over[pub_name] =
        find_replacement->first + "," + find_replacement->second;
    find_replacement = ranges::find_if(
        find_replacement + 1, ranges::end(req_split),
        [p = pub_type, pop_config, nested_con](auto r) {
          return tag_converts_to(p, r.second, pop_config, nested_con);
        });
    if (find_replacement != ranges::end(req_split)) {
      std::cout << "error: for published " << signal_category << "-signal "
                << nested_name << "'" << pub_name
                << "' - multiple Signals provided by Population<entity> are "
                   "convertible to "
                << pub_type;
      std::exit(1);
    }
  }

  return attempted_over;
}

life::configuration check_tag_overrides(bool is_pre,
                                        life::configuration::iterator user_con,
                                        life::configuration::iterator real_con,
                                        life::configuration nested_con,
                                        life::configuration pop_config) {

  auto name = user_con.key();
  auto nested_name = user_con.value()[0];
  auto reqs = is_pre ? real_con.value()[2] : real_con.value()[3];
  auto tag_category =
      is_pre ? std::string{"pre-tags"} : std::string{"post-tags"};
  auto overs = user_con.value()[1][tag_category];
  auto published = nested_con[tag_category];

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
    if (!tag_converts_to(find_pub->second, find_req->second, pop_config,
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
        pub_split, [r = req_type, pop_config, nested_con](auto p) {
          return tag_converts_to(p.second, r, pop_config, nested_con);
        });
    if (find_replacement == ranges::end(pub_split)) {
      std::cout << "error: in requirements of " << name
                << "\nno published  tags are convertible\n";
      std::exit(1);
    }
    attempted_over[find_replacement->first] = req_name + "," + req_type;
    find_replacement = ranges::find_if(
        find_replacement + 1, ranges::end(pub_split),
        [r = req_type, pop_config, nested_con](auto p) {
          return tag_converts_to(p.second, r, pop_config, nested_con);
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
                                            life::configuration env_config,
                                            life::configuration pop_config) {

  auto real_con = life::config::true_parameters(mip);

  for (auto it = env_config["parameters"].begin();
       it != env_config["parameters"].end(); it++) {
    auto rit = real_con["parameters"].find(it.key());
    if (rit == real_con["parameters"].end())
      life::config::config_mismatch_error(it.key(), mip);

    if (rit->type_name() != it->type_name())
      life::config::type_mismatch_error(it.key(), rit->type_name(),
                                        it->type_name(), mip);

    // non <environment> parameters are not checked
    if (it->type_name() != std::string{"array"} ||
        rit.value()[0] != "null_environment") {
      rit.value() = it.value();
    }
  }

  // check all <environment> parameters
  for (auto rit = real_con["parameters"].begin();
       rit != real_con["parameters"].end(); rit++) {
    if (rit->type_name() == std::string{"array"} &&
        rit.value()[0] == "null_environment") {
      auto it = env_config["parameters"].find(rit.key());
      if (it != env_config["parameters"].end()) {
        auto nested_con =
            life::config::true_parameters({"environment", it.value()[0]});
        auto pre_tags =
            check_tag_overrides(true, it, rit, nested_con, pop_config);
        auto post_tags =
            check_tag_overrides(false, it, rit, nested_con, pop_config);
        rit.value() = life::configuration::array(
            {it.value()[0],
             true_environment_object({"environment", it.value()[0]},
                                     it.value()[1], pop_config)});
        auto org_in_tags = check_org_signal_tag_overrides(
            true, it, rit.value()[1], pop_config, nested_con);
        auto org_out_tags = check_org_signal_tag_overrides(
            false, it, rit.value()[1], pop_config, nested_con);
        rit.value()[1]["pre-tags"] = pre_tags;
        rit.value()[1]["post-tags"] = post_tags;
        rit.value()[1]["org-inputs"] = org_in_tags;
        rit.value()[1]["org-outputs"] = org_out_tags;
      } else {
        check_unmentioned_tag_overrides(mip, rit.key(), rit.value());
      }
    }
  }
  return real_con;
}

void pretty_show_entity(life::ModuleInstancePair mip, life::configuration con) {
  std::cout << "\033[31m entity::" << mip.second
            << "\033[0m\n\033[33mDefault Parameters ----\033[0m\n";
  for (auto &[key, value] : con["parameters"].items())
    std::cout << std::setw(26) << key << " : " << value << "\n";
  std::cout << "\033[33m               ----\033[0m\n";
}

void pretty_show_environment(life::ModuleInstancePair mip,
                             life::configuration con) {
  std::cout << "\033[31menvironment::" << mip.second
            << "\033[0m\n\033[33mDefault Parameters ----\033[0m\n";
  for (auto &[key, value] : con["parameters"].items())
    if (value.type_name() != std::string{"array"})
      std::cout << std::setw(26) << key << " : " << value << "\n";
  std::cout << "\033[33m                   ----\033[0m\n";
  std::cout << "\033[33mPre-Tags ----\033[0m\n";
  for (auto &[key, value] : con["pre-tags"].items())
    std::cout << std::setw(26) << key << " : " << value << "\n";
  std::cout << "\033[33m         ----\033[0m\n";
  std::cout << "\033[33mPost-Tags ----\033[0m\n";
  for (auto &[key, value] : con["post-tags"].items())
    std::cout << std::setw(26) << key << " : " << value << "\n";
  std::cout << "\033[33m          ----\033[0m\n";
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
      R"~~(^\s*vary\s+([-\w\d]+)\s*=\s*\$([-\w\d]+)\s*(\{)?\s*$)~~"};
  std::regex parameter{R"~~(^\s*vary\s*([-\w\d]+)\s*=\s*([\.\-\w\d]+)\s*$)~~"};
  std::regex pre_tag{R"~~(^\s*pre\s*([-\w\d]+)\s*=\s*([-\w\d]+)\s*$)~~"};
  std::regex post_tag{R"~~(^\s*pos\s*([-\w\d]+)\s*=\s*([-\w\d]+)\s*$)~~"};
  std::regex in_signal_tag{R"~~(^\s*ist\s*([-\w\d]+)\s*=\s*([-\w\d]+)\s*$)~~"};
  std::regex out_signal_tag{R"~~(^\s*ost\s*([-\w\d]+)\s*=\s*([-\w\d]+)\s*$)~~"};

  std::ifstream ifs(file_name);
  if (!ifs.is_open()) {
    std::cout << "Error: qst file \"" << file_name << "\" does not exist\n";
    std::exit(1);
  }

  std::map<std::string, std::string> all_variables;
  struct component_spec {
    std::string variable_or_comp_name, comp, params, pres, posts, in_sigs,
        out_sigs;
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
        component_stack.push_back({m[1].str(), m[2].str(), "", "", "", "", ""});
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
        component_stack.push_back({m[1].str(), m[2].str(), "", "", "", "", ""});
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

    if (std::regex_match(line, m, in_signal_tag)) {
      std::cout << "matched in-signal-tag regex with - " << line << std::endl;
      component_stack.back().in_sigs +=
          "\"" + m[1].str() + "\":\"" + m[2].str() + "\",";
      continue;
    }

    if (std::regex_match(line, m, out_signal_tag)) {
      std::cout << "matched out-signal-tag regex with - " << line << std::endl;
      component_stack.back().out_sigs +=
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
      auto expanded_component =
          "[\"" + current.comp + "\",{\"parameters\":{" + current.params +
          "},\"pre-tags\":{" + current.pres + "},\"post-tags\":{" +
          current.posts + "},\"org-inputs\":{" + current.in_sigs +
          "},\"org-outputs\":{" + current.out_sigs + "}}]";
      if (component_stack.empty()) {
        all_variables[current.variable_or_comp_name] = expanded_component;
      } else {
        component_stack.back().params += "\"" + current.variable_or_comp_name +
                                         "\":" + expanded_component + ",";
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
  // check all things that aren't being checked statically, in particular the
  // publications of components
  check_all_configs_correct();

  if (argc == 2 && std::string(argv[1]) == "-h") {
    std::cout << "-s : saves configuration files\n"
              << "-f <file-name> : runs experiment in file-name\n";
  } else if (argc == 2 && std::string(argv[1]) == "-s") {
    std::cout << "saving configurations.cfg ... \n";
    save_configs();
  } else if (argc > 2 && std::string(argv[1]) == "-p") {
    for (auto i{2}; i < argc; i++)
      show_config(std::string(argv[i]));
    std::cout << std::endl;
  } else if (argc == 3 && std::string(argv[1]) == "-b") {
    std::string qst_path = argv[2];
    life::global_path = qst_path.substr(0, qst_path.find_last_of('/') + 1);

    std::hash<std::string> hash_fn;

    auto [pop_con, env_con] = parse_qst(qst_path);

    auto true_pop = life::configuration::array(
        {pop_con[0], true_any_object({"population", pop_con[0]}, pop_con[1])});
    auto true_env = life::configuration::array(
        {env_con[0],
         true_environment_object({"environment", env_con[0]}, env_con[1],
                                 true_pop[1]["parameters"]["entity"][1])});

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
  }
}
