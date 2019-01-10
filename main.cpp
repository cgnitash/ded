

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

life::configuration check_org_in_tag_overrides(
    std::string name, std::string nested_name, life::configuration reqs,
    life::configuration overs, life::configuration published,
    life::configuration pop_config, life::configuration env_config) {

  if (published.empty())
    return nullptr;

  std::cout << "in org-in-tag of " << name << " :required " << reqs << " "
            << reqs.size() << "\n and :published " << published << " "
            << published.size() << "\n";
  if (reqs.size() < published.size()) {
    std::cout << "error: Population cannot handle all the input-tags provided "
                 "by environment::"
              << name << "\n";
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
    if (!tag_converts_to(find_pub->second, find_req->second, pop_config,
                         env_config)) {
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

  for (auto &pub : pub_split) {
    if (ranges::count_if(req_split, [pub, pop_config, env_config](auto r) {
          return tag_converts_to(pub.second, r.second, pop_config, env_config);
        }) != 1) {
      std::cout
          << "error: ambiguity resulting from matching tags with convertible "
             "type in requirements of "
          << name << "\n";
      std::exit(1);
    }
    auto find_replacement =
        ranges::find_if(req_split, [pub, pop_config, env_config](auto r) {
          return tag_converts_to(pub.second, r.second, pop_config, env_config);
        });
    if (find_replacement == ranges::end(pub_split)) {
      std::cout << "error: this should NEVER happen" << std::endl;
      std::exit(1);
    }
    attempted_over[pub.first] =
        find_replacement->first + "," + find_replacement->second;
  }

  std::cout << "still in org-in-tags " << attempted_over << "\n";

  return attempted_over;
}

life::configuration check_org_out_tag_overrides(
    std::string name, std::string nested_name, life::configuration reqs,
    life::configuration overs, life::configuration published,
    life::configuration pop_config, life::configuration env_config) {

  if (published.empty())
    return nullptr;

  std::cout << "in org-out-tags of " << name << " :required " << reqs << " "
            << reqs.size() << "\n and :published " << published << " "
            << published.size() << "\n";
  if (reqs.size() < published.size()) {
    std::cout << "error: Population cannot handle all the output-tags provided "
                 "by environment::"
              << name << "\n";
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
    if (!tag_converts_to(find_pub->second, find_req->second, pop_config,
                         env_config)) {
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

  for (auto &pub : pub_split) {
    if (ranges::count_if(req_split, [pub, pop_config, env_config](auto r) {
          return tag_converts_to(pub.second, r.second, pop_config, env_config);
        }) != 1) {
      std::cout
          << "error: ambiguity resulting from matching tags with convertible "
             "type in requirements of "
          << name << "\n";
      std::exit(1);
    }
    auto find_replacement =
        ranges::find_if(req_split, [pub, pop_config, env_config](auto r) {
          return tag_converts_to(pub.second, r.second, pop_config, env_config);
        });
    if (find_replacement == ranges::end(pub_split)) {
      std::cout << "error: this should NEVER happen" << std::endl;
      std::exit(1);
    }
    attempted_over[pub.first] =
        find_replacement->first + "," + find_replacement->second;
  }

  std::cout << "still in org-outs " << attempted_over << "\n";
  return attempted_over;
}

life::configuration check_pre_tag_overrides(
    std::string name, std::string nested_name, life::configuration reqs,
    life::configuration overs, life::configuration published,
    life::configuration pop_config, life::configuration env_config) {

  std::cout << "in pre of " << name << " :required " << reqs << " "
            << reqs.size() << "\n and :published " << published << " "
            << published.size() << "\n";
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
    if (!tag_converts_to(find_pub->second, find_req->second, pop_config,
                         env_config)) {
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

  for (auto &req : req_split) {
    if (ranges::count_if(pub_split, [req, pop_config, env_config](auto p) {
          return tag_converts_to(p.second, req.second, pop_config, env_config);
        }) != 1) {
      std::cout
          << "error: ambiguity resulting from matching tags with convertible "
             "type in requirements of "
          << name << "\n";
      std::exit(1);
    }
    auto find_replacement =
        ranges::find_if(pub_split, [req, pop_config, env_config](auto p) {
          return tag_converts_to(p.second, req.second, pop_config, env_config);
        });
    if (find_replacement == ranges::end(pub_split)) {
      std::cout << "error: this should NEVER happen" << std::endl;
      std::exit(1);
    }
    attempted_over[find_replacement->first] = req.first + "," + req.second;
  }

  std::cout << "still in pre " << attempted_over << "\n";

  return attempted_over;
}

life::configuration check_post_tag_overrides(
    std::string name, std::string nested_name, life::configuration reqs,
    life::configuration overs, life::configuration published,
    life::configuration pop_config, life::configuration env_config) {

  std::cout << "in post of " << name << " :required " << reqs << " "
            << reqs.size() << "\n and :published " << published << " "
            << published.size() << "\n";
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
    if (!tag_converts_to(find_pub->second, find_req->second, pop_config,
                         env_config)) {
      std::cout << "error: " << find_req->first << " must be type '"
                << find_req->second << "' but " << find_pub->first
                << " has type '" << find_pub->second << "'\n";
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

  for (auto &req : req_split) {
    if (ranges::count_if(pub_split, [req, pop_config, env_config](auto p) {
          return tag_converts_to(p.second, req.second, pop_config, env_config);
        }) != 1) {
      std::cout
          << "error: ambiguity resulting from matching tags with convertible "
             "type in requirements of "
          << name << "\n";
      std::exit(1);
    }
    auto find_replacement =
        ranges::find_if(pub_split, [req, pop_config, env_config](auto p) {
          return tag_converts_to(p.second, req.second, pop_config, env_config);
        });
    if (find_replacement == ranges::end(pub_split)) {
      std::cout << "error: this should NEVER happen" << std::endl;
      std::exit(1);
    }
    attempted_over[find_replacement->first] = req.first + "," + req.second;
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
                                            life::configuration con,
                                            life::configuration pop_config) {

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
            nested_con["pre-tags"], pop_config, nested_con);
        auto post_tags = check_post_tag_overrides(
            it.key(), it.value()[0], rit.value()[3], it.value()[1]["post-tags"],
            nested_con["post-tags"], pop_config, nested_con);
        rit.value() = life::configuration::array(
            {it.value()[0],
             true_environment_object({"environment", it.value()[0]},
                                     it.value()[1], pop_config)});
        auto org_in_tags = check_org_in_tag_overrides(
            it.key(), it.value()[0], pop_config["input-tags"],
            it.value()[1]["org-inputs"], nested_con["org-inputs"], pop_config,
            rit.value()[1]);
        auto org_out_tags = check_org_out_tag_overrides(
            it.key(), it.value()[0], pop_config["output-tags"],
            it.value()[1]["org-outputs"], nested_con["org-outputs"], pop_config,
            rit.value()[1]);
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
      std::cout << "\033[32mNested Environment ----\033[0m \033[31m"
                 << key << "\033[0m\n";
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
  }
  else if (argc == 3 && std::string(argv[1]) == "-b") {
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
