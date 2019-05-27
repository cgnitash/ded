
#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <range/v3/all.hpp>
#include <string>
#include <vector>

#include "specs/entity_spec.h"
#include "specs/environment_spec.h"
#include "specs/population_spec.h"

#include "term_colours.h"

namespace life {

extern std::string                                       global_path;

using configuration      = nlohmann::json;
using ModuleInstancePair = std::pair<std::string, std::string>;
extern std::map<ModuleInstancePair, life::configuration> all_configs;
extern std::map<std::string, entity_spec>                all_entity_specs;
extern std::map<std::string, environment_spec>           all_environment_specs;
extern std::map<std::string, population_spec>            all_population_specs;

// component_builder will provide full specializations
template <typename T> std::string auto_class_name_as_string() = delete;

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

inline void
    missing_module_instance_error(life::ModuleInstancePair mip)
{
  auto &true_mod       = mip.first;
  auto &attempted_inst = mip.second;
  std::cout << "Error: Non-existent <Module>::Instance -- "
            << term_colours::red_fg << "<" << true_mod
            << ">::" << attempted_inst << "" << term_colours::reset << "\n";
  for (auto &type_name_config_pair : life::all_configs)
  {
    auto &[mod, inst] = type_name_config_pair.first;
    if (mod == true_mod && match(attempted_inst, inst))
      std::cout << "Did you mean " << term_colours::green_fg << "'" << inst
                << "'" << term_colours::reset << "?\n";
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

  std::cout << "Error: Configuration mismatch -- " << term_colours::yellow_fg
            << "<" << mip.first << ">::" << mip.second << ""
            << term_colours::reset << " does not have parameter named "
            << term_colours::red_fg << "'" << key << "'" << term_colours::reset
            << "\n";
  auto con = true_parameters(mip);
  for (auto it : con["parameters"].items())
    if (match(key, it.key()))
      std::cout << "Did you mean " << term_colours::green_fg << "'" << it.key()
                << "'" << term_colours::reset << "?\n";

  std::exit(1);
}

inline void
    type_mismatch_error(std::string              name,
                        std::string              real,
                        std::string              fake,
                        life::ModuleInstancePair mip)
{
  std::cout << "Error: Type mismatch -- " << term_colours::yellow_fg << "<"
            << mip.first << ">::" << mip.second << "'" << name << "'"
            << term_colours::reset << " must have type "
            << term_colours::green_fg << "'" << real << "'"
            << term_colours::reset << " but has type " << term_colours::red_fg
            << "'" << fake << "'" << term_colours::reset << "\n";
  std::exit(1);
}

inline auto
    signal_name_type(std::string s)
{
  auto p = s.find_first_of(',');
  return std::make_pair(s.substr(0, p), s.substr(p + 1));
}

inline void
    check_entity_correct()
{

  /*  for (auto &group : { "parameters", "input-tags", "output-tags" })
      if (config.find(group) == config.end())
      {
        std::cout << "User publication error: user module" <<
    term_colours::red_fg
                  << "<" << type_name.first << ">::'" << type_name.second << "'"
                  << term_colours::reset << " must publish "
                  << term_colours::red_fg << "'" << group << "'"
                  << term_colours::reset << " group\n";
        exit(1);
      }

    for (auto &group : { "input-tags", "output-tags" })
      for (auto &[name, type] : config[group].items())
        if (name != signal_name_type(type).first)
        {
          std::cout << "User publication error: user module"
                    << term_colours::red_fg << "<" << type_name.first << ">::'"
                    << type_name.second << "'" << term_colours::reset
                    << " must synchronise name of " << group << ":"
                    << term_colours::green_fg << "" << name << ""
                    << term_colours::reset << " with "
                    << "" << term_colours::green_fg << "" << type << ""
                    << term_colours::reset << ""
                    << "\n";
          exit(1);
        }

    if (config.size() != 3)
    {
      std::cout << "User publication error: user module" << term_colours::red_fg
                << "<" << type_name.first << ">::'" << type_name.second << "'"
                << term_colours::reset << " must not publish unspecified groups"
                << "\n";
      exit(1);
    } */
  return;
}

inline void
    check_environment_correct()
{

  /*  for (auto &group :
         { "parameters", "pre-tags", "post-tags", "input-tags", "output-tags" })
      if (config.find(group) == config.end())
      {
        std::cout << "User publication error: user module" <<
    term_colours::red_fg
                  << "<" << type_name.first << ">::'" << type_name.second << "'"
                  << term_colours::reset << " must publish "
                  << term_colours::red_fg << "'" << group << "'"
                  << term_colours::reset << " group\n";
        exit(1);
      }

    for (auto &group : { "pre-tags", "post-tags", "input-tags", "output-tags" })
      for (auto &[name, type] : config[group].items())
        if (name != signal_name_type(type).first)
        {
          std::cout << "User publication error: user module"
                    << term_colours::red_fg << "<" << type_name.first << ">::'"
                    << type_name.second << "'" << term_colours::reset
                    << " must synchronise name of " << group << ":"
                    << term_colours::green_fg << "" << name << ""
                    << term_colours::reset << " with "
                    << "" << term_colours::green_fg << "" << type << ""
                    << term_colours::reset << ""
                    << "\n";
          exit(1);
        }

    if (config.size() != 5)
    {
      std::cout << "User publication error: user module" << term_colours::red_fg
                << "<" << type_name.first << ">::'" << type_name.second << "'"
                << term_colours::reset << " must not publish unspecified groups"
                << "\n";
      exit(1);
    } */
  return;
}

inline void
    check_population_correct()
{

  /*
    if (config.find("parameters") == config.end())
    {
      std::cout << "User publication error: user module" << term_colours::red_fg
                << "<" << type_name.first << ">::'" << type_name.second << "'"
                << term_colours::reset << " must publish " <<
    term_colours::red_fg
                << "'parameters'" << term_colours::reset << " group"
                << "\n";
      exit(1);
    }

    if (config.size() != 1)
    {
      std::cout << "User publication error: user module" << term_colours::red_fg
                << "<" << type_name.first << ">::'" << type_name.second << "'"
                << term_colours::reset << " must not publish unspecified groups"
                << "\n";
      exit(1);
    } */
  return;
}

inline void
    check_all_configs_correct()
{
  // no-op ??
  // check_entity_correct();
  // check_environment_correct();
  // check_population_correct();

  /*  for (auto &[type_name, config] : life::all_configs)
    {
      if (type_name.first == "environment")
        check_environment_correct(type_name, config);
      if (type_name.first == "entity") check_entity_correct(type_name, config);
      if (type_name.first == "population")
        check_population_correct(type_name, config);
    } */
}

inline void
    check_unmentioned_tag_overrides(life::ModuleInstancePair mip,
                                    life::configuration      key,
                                    life::configuration      value)
{
  if (value[2] != nullptr)
  {
    std::cout << "error: " << term_colours::red_fg << "environment" << value[0]
              << "" << term_colours::reset
              << " does not handle the pre-tags that " << mip.second
              << "::" << key << " needs to handle\n";
    std::exit(1);
  }
  if (value[3] != nullptr)
  {
    std::cout << "error: " << term_colours::red_fg << "environment" << value[0]
              << "" << term_colours::reset
              << " does not provide the post-tags that " << mip.second
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
  if (!std::regex_match(req, m_req, agg) || !std::regex_match(pub, m_pub, agg))
  { return false; }   // pub and req type must match
  if (m_pub[1].str() != m_req[1].str())
    return false;

  // req is an unconstrained agg
  if (m_req[2].str().empty()) return true;

  // so req is constrained

  // pub is unconstrained
  if (m_pub[2].str().empty()) return false;

  auto is_number = [](auto type) {
    return ranges::all_of(type, [](auto c) { return std::isdigit(c); });
  };

  // so pub is constrained
  auto req_type = m_req[3].str();
  auto pub_type = m_pub[3].str();
  // req is constrained by number
  if (is_number(req_type))
  {
    // pub is constrained by number
    if (is_number(pub_type)) return req_type == pub_type;
    // pub is constrained by parameter
    auto pub_val = env_config["parameters"][pub_type].get<int>();
    return pub_val == std::stoi(req_type);
  }
  // req is constrained by parameter
  auto req_val = pop_config["parameters"][req_type].get<int>();
  // pub is constrained by number
  if (is_number(pub_type)) return std::stoi(pub_type) == req_val;
  // pub is constrained by parameter
  auto pub_val = env_config["parameters"][pub_type].get<int>();
  return pub_val == req_val;
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
    std::cout << "error: " << term_colours::red_fg << "<entity>::" << pop[0]
              << "" << term_colours::reset << " cannot handle all the "
              << signal_category << "-tags provided by " << term_colours::red_fg
              << "<environment>::" << nested_name << "" << term_colours::reset
              << "<\n";
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
      std::cout << "error: " << term_colours::red_fg
                << "<environment>::" << nested_name << "" << term_colours::reset
                << " does not have an " << signal_category << "-signal named "
                << term_colours::red_fg << "\"" << key << "\""
                << term_colours::reset
                << " to override. Please check this component's "
                   "publication\n";
      for (auto it : env_config[signal_category].items())
        if (life::config_manager::match(key, it.key()))
          std::cout << "Did you mean " << term_colours::green_fg << "'"
                    << it.key() << "'" << term_colours::reset << "?\n";
      std::exit(1);
    }
    auto find_req = ranges::find(req_split,
                                 std::string{ value },
                                 &std::pair<std::string, std::string>::first);
    if (find_req == ranges::end(req_split))
    {
      std::cout << "error: " << term_colours::red_fg << "<entity>::" << pop[0]
                << "" << term_colours::reset << " does not publish an "
                << signal_category << "-signal named " << term_colours::red_fg
                << "" << value << "" << term_colours::reset
                << ". This cannot be overridden. Please check this "
                   "component's publication\n";
      for (auto it : pop[1][signal_category].items())
        if (life::config_manager::match(value, it.key()))
          std::cout << "Did you mean " << term_colours::green_fg << "'"
                    << it.key() << "'" << term_colours::reset << "?\n";
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
      std::cout << "error: for published " << signal_category << "-signal in "
                << term_colours::red_fg << "<environment>::" << nested_name
                << "::" << pub_name << "" << term_colours::reset
                << " - No signals provided by " << term_colours::red_fg
                << "<entity>::" << pop[0] << "" << term_colours::reset
                << " are convertible to " << term_colours::yellow_fg << ""
                << pub_name << " ~ " << pub_type << "" << term_colours::reset
                << "";
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
      std::cout << "error: for published " << signal_category << "-signal in "
                << term_colours::red_fg << "<environment>::" << nested_name
                << "::" << pub_name << "" << term_colours::reset
                << "\n - Multiple Signals provided by " << term_colours::red_fg
                << "<entity>::" << pop[0] << "" << term_colours::reset
                << " are convertible to " << term_colours::yellow_fg << ""
                << pub_name << " ~ " << pub_type << "" << term_colours::reset
                << "\n Viable candidates are \n";
      ranges::for_each(
          req_split,
          [p = pub_type, pop_config = pop[1], nested_con, is_input](auto r) {
            if (tag_converts_to(is_input, p, r.second, pop_config, nested_con))
              std::cout << "" << term_colours::yellow_fg << "      " << r.first
                        << " ~ " << r.second << "" << term_colours::reset << ""
                        << std::endl;
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
    pretty_show_entity(std::ostream &out, entity_spec e)
{

  out << term_colours::red_fg << "entity::" << e.name() << term_colours::reset
      << "\n";

  out << term_colours::yellow_fg << "parameters----" << term_colours::reset
      << "\n";
  for (auto [parameter, value] : e.parameters())
    out << std::setw(26) << parameter << " : " << value.value_as_string()
        << "\n";
  out << term_colours::yellow_fg << "inputs----" << term_colours::reset << "\n";
  for (auto [input, value] : e.inputs())
    out << std::setw(26) << input << " : " << value << "\n";
  out << term_colours::yellow_fg << "outputs----" << term_colours::reset
      << "\n";
  for (auto [output, value] : e.outputs())
    out << std::setw(26) << output << " : " << value << "\n";

  /*  for (std::string group : { "parameters", "input-tags", "output-tags" })
    {
      std::cout << "" << term_colours::yellow_fg << "" << group << "----"
                << term_colours::reset << "\n";
      for (auto &[key, value] : con[group].items())
        if (value.type_name() != std::string{ "array" })
          std::cout << std::setw(26) << key << " : " << value << "\n";
      std::cout << "" << term_colours::yellow_fg << ""
                << std::string(group.length(), ' ') << "----"
                << term_colours::reset << "\n";
    } */
  return;
}

inline void
    pretty_show_environment(std::ostream &out, environment_spec e)
{

  out << term_colours::red_fg << "environment::" << e.name()
      << term_colours::reset << "\n";

  out << term_colours::yellow_fg << "parameters----" << term_colours::reset
      << "\n";
  for (auto [parameter, value] : e.parameters())
    out << std::setw(26) << parameter << " : " << value.value_as_string()
        << "\n";
  out << term_colours::yellow_fg << "inputs----" << term_colours::reset << "\n";
  for (auto [input, value] : e.inputs())
    out << std::setw(26) << input << " : " << value << "\n";
  out << term_colours::yellow_fg << "outputs----" << term_colours::reset
      << "\n";
  for (auto [output, value] : e.outputs())
    out << std::setw(26) << output << " : " << value << "\n";
  out << term_colours::yellow_fg << "pre-tags----" << term_colours::reset
      << "\n";
  for (auto [pre_tag, value] : e.pre_tags())
    out << std::setw(26) << pre_tag << " : " << value << "\n";
  out << term_colours::yellow_fg << "post_tags----" << term_colours::reset
      << "\n";
  for (auto [post_tag, value] : e.post_tags())
    out << std::setw(26) << post_tag << " : " << value << "\n";

  out << term_colours::yellow_fg << "nested----" << term_colours::reset << "\n";
  for (auto &[name, nspec] : e.nested())
  {
    out << std::setw(26) << name << " :\n";
    if (nspec.pre_constraints.empty())
      out << "No pre-constraints\n";
    else
    {
      out << "pre-constraints:\n";
      for (auto name : nspec.pre_constraints)
        out << std::setw(26) << name << " :\n";
    }
    if (nspec.post_constraints.empty())
      out << "No post-constraints\n";
    else
    {
      out << "post-constraints:\n";
      for (auto name : nspec.post_constraints)
        out << std::setw(26) << name << " :\n";
    }
  }

  if (auto tfc = e.tag_flow_constraints(); !tfc.empty())
  {
    out << "with tag-flow-constraints:\n";
    for (auto name : tfc)
      out << std::setw(26) << name.first.second << "(" << name.first.first
          << ") <=> " << name.second.second << "(" << name.second.first
          << ")\n";
  }

  /* std::cout << "" << term_colours::red_fg << "environment::" << mip.second <<
    ""
              << term_colours::reset << "\n";

    for (std::string group :
         { "parameters", "pre-tags", "post-tags", "input-tags", "output-tags" })
    {
      std::cout << "" << term_colours::yellow_fg << "" << group << "----"
                << term_colours::reset << "\n";
      for (auto &[key, value] : con[group].items())
        if (value.type_name() != std::string{ "array" })
          std::cout << std::setw(26) << key << " : " << value << "\n";
      std::cout << "" << term_colours::yellow_fg << ""
                << std::string(group.length(), ' ') << "----"
                << term_colours::reset << "\n";
    }

    for (auto &[key, value] : con["parameters"].items())
      if (value.type_name() == std::string{ "array" } &&
          value[0] == "null_environment")
      {
        std::cout << "" << term_colours::green_fg << "Nested Environment ----"
                  << term_colours::reset << " " << term_colours::red_fg << ""
                  << key << "" << term_colours::reset << "\n";
        if (value[2] != nullptr)
          std::cout << "with pre-tag requirements " << term_colours::green_fg
                    << "" << value[2] << "" << term_colours::reset << "\n";
        if (value[3] != nullptr)
          std::cout << "with post-tag requirements " << term_colours::green_fg
                    << "" << value[3] << "" << term_colours::reset << "\n";
        std::cout << "" << term_colours::green_fg << "                   ----"
                  << term_colours::reset << "\n";
      } */
  return;
}

inline void
    pretty_show_population(std::ostream &out, population_spec &e)
{
  out << term_colours::red_fg << "population::" << e.name()
      << term_colours::reset << "\n";

  out << term_colours::yellow_fg << "parameters----" << term_colours::reset
      << "\n";
  for (auto [parameter, value] : e.parameters())
    out << std::setw(26) << parameter << " : " << value.value_as_string()
        << "\n";
  out << term_colours::yellow_fg << "inputs----" << term_colours::reset << "\n";
  for (auto [input, value] : e.inputs())
    out << std::setw(26) << input << " : " << value << "\n";
  out << term_colours::yellow_fg << "outputs----" << term_colours::reset
      << "\n";
  for (auto [output, value] : e.outputs())
    out << std::setw(26) << output << " : " << value << "\n";

  /*
  std::cout << "" << term_colours::red_fg << "population::" << mip.second << ""
            << term_colours::reset << "\n"
            << term_colours::yellow_fg << "Default Parameters ----"
            << term_colours::reset << "\n";
  for (auto &[key, value] : con["parameters"].items())
    std::cout << std::setw(26) << key << " : " << value << "\n";
  std::cout << "" << term_colours::yellow_fg << "               ----"
            << term_colours::reset << "\n";
                        */
  return;
}

inline void
    show_config(std::ostream &out, std::string name)
{
  auto found = false;
  if (life::all_entity_specs.find(name) != life::all_entity_specs.end())
  {
    pretty_show_entity(out, life::all_entity_specs[name]);
    found = true;
  }
  if (life::all_environment_specs.find(name) !=
      life::all_environment_specs.end())
  {
    pretty_show_environment(out, life::all_environment_specs[name]);
    found = true;
  }
  if (life::all_population_specs.find(name) != life::all_population_specs.end())
  {
    pretty_show_population(out, life::all_population_specs[name]);
    found = true;
  }

  if(!found)
    out << "component " << name << " not found\n";

  /*
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
        std::cout << "Did you mean " << term_colours::green_fg << "<"
                  << type_name_config.first.first
                  << ">::" << type_name_config.first.second << ""
                  << term_colours::reset << "?\n";
  }
  */
  return;
}

inline void
    save_configs()
{

  std::ofstream file("configurations.cfg");
  for (auto [name, es] : life::all_entity_specs) pretty_show_entity(file, es);
  for (auto [name, es] : life::all_environment_specs)
    pretty_show_environment(file, es);
  for (auto [name, es] : life::all_population_specs)
    pretty_show_population(file, es);
  /*
  for (auto &[type_name, config] : life::all_configs)
    file << "\n<" << type_name.first << ">::" << type_name.second << "\n  "
         << std::setw(4) << config << std::endl;
                 */
  return;
}

}   // namespace config_manager
}   // namespace life
