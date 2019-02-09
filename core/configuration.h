
#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <range/v3/all.hpp>
#include <string>
#include <vector>

namespace life {

using configuration      = nlohmann::json;
using ModuleInstancePair = std::pair<std::string, std::string>;
extern std::map<ModuleInstancePair, life::configuration> all_configs;
extern std::string global_path;

template<typename T>
	std::string  auto_class_name_as_string() { return "oops";}

// almost certainly unnecessary
inline void
    validate_subset(const configuration &in, const configuration &real)
{
  if (in.find("parameters") == in.end()) return;
  const auto &in_params   = in["parameters"];
  const auto &real_params = real["parameters"];
  for (auto it = in_params.begin(); it != in_params.end(); ++it)
  {
    if (real_params.find(it.key()) == real_params.end())
    {
      std::cout << "Error: Configuration mismatch -- \"" << it.key()
                << "\" is not a valid parameter\n";
      exit(1);
    }
  }
}

// almost certainly unnecessary
inline void
    merge_into(configuration &in, const configuration &real)
{
  if (in.find("parameters") == in.end())
  {
    in = real;
  } else
  {
    auto &      in_params   = in["parameters"];
    const auto &real_params = real["parameters"];
    for (const auto &[key, value] : real_params.items())
      if (in_params.find(key) == in_params.end()) in_params[key] = value;
  }
}

namespace config_manager {

inline auto
    closeness(std::string w1, std::string w2)
{
  return ranges::inner_product(
      w1, w2, 0, std::plus<int>(), [](auto c1, auto c2) {
        return c1 != c2 ? 1 : 0;
      });
}

inline auto
    min_max_distance(std::string w1, std::string w2)
{
  int l1 = w1.length();
  int l2 = w2.length();

  if (l1 < l2) return std::make_tuple(w1, w2, l2 - l1);
  if (l1 > l2) return std::make_tuple(w2, w1, l1 - l2);
  return std::make_tuple(w1, w2, 0);
}

inline auto
    match(std::string attempt, std::string word)
{
  // In : abc
  // Out : [" abc","a bc","ab c","abc "]
  auto all_spaces = [](std::string word) {
    return ranges::view::repeat_n(word, word.size() + 1) |
           ranges::view::transform([n = -1](auto str) mutable {
             n++;
             return str.substr(0, n) + " " + str.substr(n);
           });
  };

  auto tolerance            = 3;
  auto [min, max, distance] = min_max_distance(attempt, word);

  if (distance == 0)
  {
    // at most deletion + insertion, or at most 2 changes
    for (auto space_one : all_spaces(min))
      for (auto space_two : all_spaces(max))
        if (closeness(space_one, space_two) < tolerance) return true;
  }

  if (distance == 1)
  {
    // at most symmetrically (deletion + change, or insertion + change)
    for (auto spaced : all_spaces(min))
      if (closeness(spaced, max) < tolerance) return true;
  }
  if (distance == 2)
  {
    // at most symmetrically ( 2 deletions , or 2 insertions)
    for (auto spaced :
         all_spaces(min) | ranges::view::transform([all_spaces](auto r) {
           return all_spaces(r);
         }) | ranges::view::join)
      if (closeness(spaced, max) < tolerance) return true;
  }

  // if (distance >= tolerance)
  return false;
}

inline auto
    missing_module_instance_error(life::ModuleInstancePair mip)
{
  auto &true_mod       = mip.first;
  auto &attempted_inst = mip.second;
  std::cout << "Error: Non-existent <Module>::Instance -- \033[31m<" << true_mod
            << ">::" << attempted_inst << "\033[0m\n";
  for (auto &type_name_config_pair : life::all_configs)
  {
    auto &[mod, inst] = type_name_config_pair.first;
    if (mod == true_mod && match(attempted_inst, inst))
      std::cout << "Did you mean \033[32m'" << inst << "'\033[0m?\n";
  }
  std::exit(1);
}

inline life::configuration
    true_parameters(life::ModuleInstancePair mip)
{
  auto real_con_it = life::all_configs.find(mip);
  if (life::all_configs.end() == real_con_it)
    missing_module_instance_error(mip);
  return real_con_it->second;
}

inline void
    config_mismatch_error(std::string key, life::ModuleInstancePair mip)
{

  std::cout << "Error: Configuration mismatch -- \033[33m<" << mip.first
            << ">::" << mip.second
            << "\033[0m does not have parameter named \033[31m'" << key
            << "'\033[0m\n";
  auto con = true_parameters(mip);
  for (auto it : con["parameters"].items())
    if (match(key, it.key()))
      std::cout << "Did you mean \033[32m'" << it.key() << "'\033[0m?\n";

  std::exit(1);
}

inline void
    type_mismatch_error(std::string              name,
                        std::string              real,
                        std::string              fake,
                        life::ModuleInstancePair mip)
{
  std::cout << "Error: Type mismatch -- \033[33m<" << mip.first
            << ">::" << mip.second << "'" << name
            << "'\033[0m must have type \033[32m'" << real
            << "'\033[0m but has type \033[31m'" << fake << "'\033[0m\n";
  std::exit(1);
}

inline void
    check_environment_correct(life::ModuleInstancePair type_name,
                              life::configuration      config)
{

  for (auto &group :
       { "parameters", "pre-tags", "post-tags", "input-tags", "output-tags" })
    if (config.find(group) == config.end())
    {
      std::cout << "User publication error: user module\033[31m<"
                << type_name.first << ">::'" << type_name.second
                << "'\033[0m must publish \033[31m'" << group
                << "'\033[0m group\n";
      exit(1);
    }

  if (config.size() != 5)
  {
    std::cout << "User publication error: user module\033[31m<"
              << type_name.first << ">::'" << type_name.second
              << "'\033[0m must not publish unspecified groups"
              << "\n";
    exit(1);
  }
  return;
}

inline void
    check_population_correct(life::ModuleInstancePair type_name,
                             life::configuration      config)
{
  if (config.find("parameters") == config.end())
  {
    std::cout << "User publication error: user module\033[31m<"
              << type_name.first << ">::'" << type_name.second
              << "'\033[0m must publish \033[31m'parameters'\033[0m group"
              << "\n";
    exit(1);
  }

  if (config.size() != 1)
  {
    std::cout << "User publication error: user module\033[31m<"
              << type_name.first << ">::'" << type_name.second
              << "'\033[0m must not publish unspecified groups"
              << "\n";
    exit(1);
  }
  return;
}

inline void
    check_entity_correct(life::ModuleInstancePair type_name,
                         life::configuration      config)
{

  for (auto &group : { "parameters", "input-tags", "output-tags" })
    if (config.find(group) == config.end())
    {
      std::cout << "User publication error: user module\033[31m<"
                << type_name.first << ">::'" << type_name.second
                << "'\033[0m must publish \033[31m'" << group
                << "'\033[0m group\n";
      exit(1);
    }

  if (config.size() != 3)
  {
    std::cout << "User publication error: user module\033[31m<"
              << type_name.first << ">::'" << type_name.second
              << "'\033[0m must not publish unspecified groups"
              << "\n";
    exit(1);
  }
  return;
}

inline void
    check_all_configs_correct()
{

  for (auto &[type_name, config] : life::all_configs)
  {
    if (type_name.first == "environment")
      check_environment_correct(type_name, config);
    if (type_name.first == "entity") check_entity_correct(type_name, config);
    if (type_name.first == "population")
      check_population_correct(type_name, config);
  }
}

inline auto
    signal_name_type(std::string s)
{
  auto p = s.find_first_of(',');
  return std::make_pair(s.substr(0, p), s.substr(p + 1));
}

inline void
    check_unmentioned_tag_overrides(life::ModuleInstancePair mip,
                                    life::configuration      key,
                                    life::configuration      value)
{
  if (value[2] != nullptr)
  {
    std::cout << "error: \033[31menvironment" << value[0]
              << "\033[0m does not handle the pre-tags that " << mip.second
              << "::" << key << " needs to handle\n";
    std::exit(1);
  }
  if (value[3] != nullptr)
  {
    std::cout << "error: \033[31menvironment" << value[0]
              << "\033[0m does not provide the post-tags that " << mip.second
              << "::" << key << " needs to provide\n";
    std::exit(1);
  }
}

inline bool
    tag_converts_to(bool                in_order,
                    std::string         pub,
                    std::string         req,
                    life::configuration pop_config,
                    life::configuration env_config)
{

  if (!in_order)
  {
    std::swap(pub, req);
    std::swap(pop_config, env_config);
  }

  // req matches absolutely any input
  if (req.empty()) return true;

  std::regex pod{ R"~~(^(double|long|bool)$)~~" };
  std::regex agg{ R"~~(^A\<(double|long|bool)(,([-\w\d]+))?\>$)~~" };

  std::smatch m_req, m_pub;
  // pub and req are pods
  if (std::regex_match(pub, m_pub, pod) && std::regex_match(req, m_req, pod))
  { return pub == req; }   // exactly one of pub or req are pods
  if (std::regex_match(pub, m_pub, pod) || std::regex_match(req, m_req, pod))
  { return false; }   // both pub and req must be aggs
  if (std::regex_match(req, m_req, agg) && std::regex_match(pub, m_pub, agg))
  {
    // pub and req type must match
    if (m_pub[1].str() != m_req[1].str()) return false;

    // req is an unconstrained agg
    if (m_req[2].str().empty()) return true;

    // so req is constrained

    // pub is unconstrained
    if (m_pub[2].str().empty()) return false;

    // so pub is constrained
    auto req_type = m_req[3].str();
    auto pub_type = m_pub[3].str();
    // req is constrained by number
    if (ranges::all_of(req_type, [](auto c) { return std::isdigit(c); }))
    {
      // pub is constrained by number
      if (ranges::all_of(pub_type, [](auto c) { return std::isdigit(c); }))
        return req_type == pub_type;
      // pub is constrained by parameter
      auto j_pub_val = env_config["parameters"][pub_type];
      auto pub_val   = j_pub_val.get<int>();
      return pub_val == std::stoi(req_type);
    }
    // req is constrained by parameter
    auto j_req_val = pop_config["parameters"][req_type];
    auto req_val   = j_req_val.get<int>();
    // pub is constrained by number
    if (ranges::all_of(pub_type, [](auto c) { return std::isdigit(c); }))
      return std::stoi(pub_type) == req_val;
    // pub is constrained by parameter
    auto j_pub_val = env_config["parameters"][pub_type];
    auto pub_val   = j_pub_val.get<int>();
    return pub_val == req_val;
  }
  return false;
}

inline life::configuration
    check_org_signal_tag_overrides(bool                          is_input,
                                   life::configuration::iterator it,
                                   life::configuration::iterator rit,
                                   life::configuration           pop,
                                   life::configuration           env_config)
{

  auto signal_category =
      is_input ? std::string{ "input-tags" } : std::string{ "output-tags" };
  auto name        = it.key();
  auto nested_name = it.value()[0];
  auto reqs        = pop[1][signal_category];
  auto nested_con  = rit.value()[1];
  auto overs       = it.value()[1][signal_category];
  auto published   = env_config[signal_category];

  /*
 std::cout << "reqs" << reqs << std::endl;
 std::cout << "overs" << overs << std::endl;
 std::cout << "pop_con" << pop << std::endl;
 std::cout << "env_con" << env_config << std::endl;
 std::cout << "it" << it.value()[1] << std::endl;
 std::cout << "published" << published << std::endl;
*/

  if (reqs.size() < published.size())
  {
    std::cout << "error: \033[31m<entity>::" << pop[0]
              << "\033[0m cannot handle all the " << signal_category
              << "-tags provided by \033[31m<environment>::" << nested_name
              << "\033[0m<\n";
    exit(1);
  }

  std::vector<std::pair<std::string, std::string>> req_split;
  for (auto &r : reqs)
    req_split.push_back(life::config_manager::signal_name_type(r));

  // try ::as_lvalue  ??
  std::vector<std::pair<std::string, std::string>> pub_split;
  for (auto &it : published.items())
    pub_split.push_back(life::config_manager::signal_name_type(it.value()));

  life::configuration attempted_over = published;

  for (auto &[key, value] : overs.items())
  {
    auto find_pub = ranges::find(
        pub_split, key, &std::pair<std::string, std::string>::first);
    if (find_pub == ranges::end(pub_split))
    {
      std::cout << "error: \033[31m<environment>::" << nested_name
                << "\033[0m does not have an " << signal_category
                << "-signal named \033[31m\"" << key
                << "\"\033[0m to override. Please check this component's "
                   "publication\n";
      for (auto it : env_config[signal_category].items())
        if (life::config_manager::match(key, it.key()))
          std::cout << "Did you mean \033[32m'" << it.key() << "'\033[0m?\n";
      std::exit(1);
    }
    auto find_req = ranges::find(req_split,
                                 std::string{ value },
                                 &std::pair<std::string, std::string>::first);
    if (find_req == ranges::end(req_split))
    {
      std::cout << "error: \033[31m<entity>::" << pop[0]
                << "\033[0m does not publish an " << signal_category
                << "-signal named \033[31m" << value
                << "\033[0m. This cannot be overridden. Please check this "
                   "component's publication\n";
      for (auto it : pop[1][signal_category].items())
        if (life::config_manager::match(value, it.key()))
          std::cout << "Did you mean \033[32m'" << it.key() << "'\033[0m?\n";
      std::exit(1);
    }
    if (!tag_converts_to(
            is_input, find_pub->second, find_req->second, pop[1], nested_con))
    {
      // should use life::config_manager::type_mismatch_error, but control flow
      // to this block is untested
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
  for (auto &[pub_name, pub_type] : pub_split)
  {
    auto find_replacement = ranges::find_if(
        req_split,
        [p = pub_type, pop_config = pop[1], nested_con, is_input](auto r) {
          return tag_converts_to(is_input, p, r.second, pop_config, nested_con);
        });
    if (find_replacement == ranges::end(req_split))
    {
      std::cout << "error: for published " << signal_category
                << "-signal in \033[31m<environment>::" << nested_name
                << "::" << pub_name
                << "\033[0m - No signals provided by \033[31m<entity>::"
                << pop[0] << "\033[0m are convertible to \033[33m" << pub_name
                << " ~ " << pub_type << "\033[0m";
      std::exit(1);
    }
    attempted_over[pub_name] =
        find_replacement->first + "," + find_replacement->second;
    find_replacement = ranges::find_if(
        find_replacement + 1,
        ranges::end(req_split),
        [p = pub_type, pop_config = pop[1], nested_con, is_input](auto r) {
          return tag_converts_to(is_input, p, r.second, pop_config, nested_con);
        });
    if (find_replacement != ranges::end(req_split))
    {
      std::cout << "error: for published " << signal_category
                << "-signal in \033[31m<environment>::" << nested_name
                << "::" << pub_name
                << "\033[0m\n - Multiple Signals provided by \033[31m<entity>::"
                << pop[0] << "\033[0m are convertible to \033[33m" << pub_name
                << " ~ " << pub_type << "\033[0m\n Viable candidates are \n";
      ranges::for_each(
          req_split,
          [p = pub_type, pop_config = pop[1], nested_con, is_input](auto r) {
            if (tag_converts_to(is_input, p, r.second, pop_config, nested_con))
              std::cout << "\033[33m      " << r.first << " ~ " << r.second
                        << "\033[0m" << std::endl;
          });
      std::exit(1);
    }
  }

  return attempted_over;
}

inline life::configuration
    check_tag_overrides(bool                          is_pre,
                        life::configuration::iterator user_it,
                        life::configuration::iterator real_it,
                        life::configuration           pop,
                        life::configuration           nested_con)
{

  auto name        = user_it.key();
  auto nested_name = user_it.value()[0];
  auto reqs        = is_pre ? real_it.value()[2] : real_it.value()[3];
  auto tag_category =
      is_pre ? std::string{ "pre-tags" } : std::string{ "post-tags" };
  auto overs     = user_it.value()[1][tag_category];
  auto published = nested_con[tag_category];

  /*
  std::cout << "reqs" << reqs << std::endl;
  std::cout << "overs" << overs << std::endl;
  std::cout << "pop_con" << pop << std::endl;
  std::cout << "env_con" << nested_con << std::endl;
  std::cout << "it" << user_it.value()[1] << std::endl;
  std::cout << "published" << published << std::endl;
  */

  if (reqs.size() != published.size())
  {
    std::cout << "error: environment::" << name
              << " cannot handle/provide all the tags supplied/needed by "
                 "environment::"
              << nested_name << "\n";
    exit(1);
  }

  std::vector<std::pair<std::string, std::string>> req_split;
  for (auto &r : reqs)
    req_split.push_back(life::config_manager::signal_name_type(r));

  // try ::as_lvalue  ??
  std::vector<std::pair<std::string, std::string>> pub_split;
  for (auto &it : published.items())
    pub_split.push_back(life::config_manager::signal_name_type(it.value()));

  life::configuration attempted_over = published;

  // apply override tags
  for (auto &[key, value] : overs.items())
  {
    auto find_pub = ranges::find(
        pub_split, key, &std::pair<std::string, std::string>::first);
    if (find_pub == ranges::end(pub_split))
    {
      std::cout << "error: " << nested_name << " does not require a "
                << tag_category << " named '" << key << "'\n";
      std::exit(1);
    }
    auto find_req = ranges::find(req_split,
                                 std::string{ value },
                                 &std::pair<std::string, std::string>::first);
    if (find_req == ranges::end(req_split))
    {
      std::cout << "error: " << nested_name << " does not publish a "
                << tag_category << " named '" << value << "'\n";
      std::exit(1);
    }
    if (!tag_converts_to(
            is_pre, find_pub->second, find_req->second, pop[1], nested_con))
    {
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
  for (auto &[req_name, req_type] : req_split)
  {
    auto find_replacement = ranges::find_if(
        pub_split,
        [r = req_type, pop_config = pop[1], nested_con, is_pre](auto p) {
          return tag_converts_to(is_pre, p.second, r, pop_config, nested_con);
        });
    if (find_replacement == ranges::end(pub_split))
    {
      std::cout << "error: in requirements of " << name
                << "\nno published  tags are convertible\n";
      std::exit(1);
    }
    attempted_over[find_replacement->first] = req_name + "," + req_type;
    find_replacement                        = ranges::find_if(
        find_replacement + 1,
        ranges::end(pub_split),
        [r = req_type, pop_config = pop[1], nested_con, is_pre](auto p) {
          return tag_converts_to(is_pre, p.second, r, pop_config, nested_con);
        });
    if (find_replacement != ranges::end(pub_split))
    {
      std::cout << "error: ambiguity in requirements of " << name
                << "\nmultiple published  tags are convertible\n";
      std::exit(1);
    }
  }

  return attempted_over;
}

inline life::configuration
    true_any_object(life::ModuleInstancePair mip, life::configuration con)
{

  auto real_con = life::config_manager::true_parameters(mip);

  for (auto it = con["parameters"].begin(); it != con["parameters"].end(); it++)
  {
    auto rit = real_con["parameters"].find(it.key());
    if (rit == real_con["parameters"].end())
      life::config_manager::config_mismatch_error(it.key(), mip);

    if (rit->type_name() != it->type_name())
      life::config_manager::type_mismatch_error(
          it.key(), rit->type_name(), it->type_name(), mip);

    rit.value() =
        it->type_name() == std::string{ "array" }
            ? life::configuration::array(
                  { it.value()[0],
                    true_any_object({ std::string{ rit.value()[0] }.substr(5),
                                      it.value()[0] },
                                    it.value()[1]) })
            : it.value();
  }
  return real_con;
}

inline life::configuration
    true_environment_object(life::ModuleInstancePair mip,
                            life::configuration      user_env_config,
                            life::configuration      user_pop)
{

  auto real_con = true_parameters(mip);

  for (auto user_it = user_env_config["parameters"].begin();
       user_it != user_env_config["parameters"].end();
       user_it++)
  {
    auto real_it = real_con["parameters"].find(user_it.key());
    if (real_it == real_con["parameters"].end())
      config_mismatch_error(user_it.key(), mip);

    if (real_it->type_name() != user_it->type_name())
      type_mismatch_error(
          user_it.key(), real_it->type_name(), user_it->type_name(), mip);

    // non <environment> parameters are not checked
    if (user_it->type_name() != std::string{ "array" } ||
        real_it.value()[0] != "null_environment")
    { real_it.value() = user_it.value(); } }

  // check all <environment> parameters
  for (auto real_it = real_con["parameters"].begin();
       real_it != real_con["parameters"].end();
       real_it++)
  {
    if (real_it->type_name() == std::string{ "array" } &&
        real_it.value()[0] == "null_environment")
    {
      auto user_it = user_env_config["parameters"].find(real_it.key());
      if (user_it != user_env_config["parameters"].end())
      {
        auto nested_env_config = life::config_manager::true_parameters(
            { "environment", user_it.value()[0] });
        auto pre_tags = check_tag_overrides(
            true, user_it, real_it, user_pop, nested_env_config);
        auto post_tags = check_tag_overrides(
            false, user_it, real_it, user_pop, nested_env_config);
        real_it.value() = life::configuration::array(
            { user_it.value()[0],
              true_environment_object({ "environment", user_it.value()[0] },
                                      user_it.value()[1],
                                      user_pop) });
        auto org_in_tags = check_org_signal_tag_overrides(
            true, user_it, real_it, user_pop, nested_env_config);
        auto org_out_tags = check_org_signal_tag_overrides(
            false, user_it, real_it, user_pop, nested_env_config);
        real_it.value()[1]["pre-tags"]    = pre_tags;
        real_it.value()[1]["post-tags"]   = post_tags;
        real_it.value()[1]["input-tags"]  = org_in_tags;
        real_it.value()[1]["output-tags"] = org_out_tags;
      } else
      {
        check_unmentioned_tag_overrides(mip, real_it.key(), real_it.value());
      }
    }
  }
  return real_con;
}

inline void
    pretty_show_entity(life::ModuleInstancePair mip, life::configuration con)
{

  std::cout << "\033[31mentity::" << mip.second << "\033[0m\n";

  for (std::string group : { "parameters", "input-tags", "output-tags" })
  {
    std::cout << "\033[33m" << group << "----\033[0m\n";
    for (auto &[key, value] : con[group].items())
      if (value.type_name() != std::string{ "array" })
        std::cout << std::setw(26) << key << " : " << value << "\n";
    std::cout << "\033[33m" << std::string(group.length(), ' ')
              << "----\033[0m\n";
  }
}

inline void
    pretty_show_environment(life::ModuleInstancePair mip,
                            life::configuration      con)
{

  std::cout << "\033[31menvironment::" << mip.second << "\033[0m\n";

  for (std::string group :
       { "parameters", "pre-tags", "post-tags", "input-tags", "output-tags" })
  {
    std::cout << "\033[33m" << group << "----\033[0m\n";
    for (auto &[key, value] : con[group].items())
      if (value.type_name() != std::string{ "array" })
        std::cout << std::setw(26) << key << " : " << value << "\n";
    std::cout << "\033[33m" << std::string(group.length(), ' ')
              << "----\033[0m\n";
  }

  for (auto &[key, value] : con["parameters"].items())
    if (value.type_name() == std::string{ "array" } &&
        value[0] == "null_environment")
    {
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

inline void
    pretty_show_population(life::ModuleInstancePair mip,
                           life::configuration      con)
{
  std::cout << "\033[31mpopulation::" << mip.second
            << "\033[0m\n\033[33mDefault Parameters ----\033[0m\n";
  for (auto &[key, value] : con["parameters"].items())
    std::cout << std::setw(26) << key << " : " << value << "\n";
  std::cout << "\033[33m               ----\033[0m\n";
}

inline void
    show_config(std::string name)
{

  auto found = false;
  for (auto &[type_name, config] : life::all_configs)
  {
    if (type_name.second == name)
    {
      found = true;
      if (type_name.first == "environment")
        pretty_show_environment(type_name, config);
      if (type_name.first == "population")
        pretty_show_population(type_name, config);
      if (type_name.first == "entity") pretty_show_entity(type_name, config);
    }
  }
  if (!found)
  {
    std::cout << "component " << name << " not found\n";
    for (auto &type_name_config : life::all_configs)
      if (life::config_manager::match(name, type_name_config.first.second))
        std::cout << "Did you mean \033[32m<" << type_name_config.first.first
                  << ">::" << type_name_config.first.second << "\033[0m?\n";
  }
}

inline void
    save_configs()
{

  std::ofstream file("configurations.cfg");
  for (auto &[type_name, config] : life::all_configs)
    file << "\n<" << type_name.first << ">::" << type_name.second << "\n  "
         << std::setw(4) << config << std::endl;
}

}   // namespace config_manager
}   // namespace life
