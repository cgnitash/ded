
#pragma once

#include <fstream>
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

inline void validate_subset(const configuration &in, const configuration &real)
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

inline void merge_into(configuration &in, const configuration &real)
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

namespace config {

inline auto closeness(std::string w1, std::string w2)
{

  return ranges::inner_product(
      w1, w2, 0, std::plus<int>(), [](auto c1, auto c2) {
        return c1 != c2 ? 1 : 0;
      });
}

inline auto min_max_distance(std::string w1, std::string w2)
{

  int l1 = w1.length();
  int l2 = w2.length();

  if (l1 < l2) return std::make_tuple(w1, w2, l2 - l1);
  if (l1 > l2) return std::make_tuple(w2, w1, l1 - l2);
  return std::make_tuple(w1, w2, 0);
}

inline auto match(std::string attempt, std::string word)
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

inline auto missing_module_instance_error(life::ModuleInstancePair mip)
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

inline life::configuration true_parameters(life::ModuleInstancePair mip)
{
  auto real_con_it = life::all_configs.find(mip);
  if (life::all_configs.end() == real_con_it)
    missing_module_instance_error(mip);
  return real_con_it->second;
}

inline void config_mismatch_error(std::string key, life::ModuleInstancePair mip)
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

inline void type_mismatch_error(std::string              name,
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

inline life::configuration true_object(life::ModuleInstancePair mip,
                                       life::configuration      con)
{

  auto real_con = true_parameters(mip);

  for (auto it = con.begin(); it != con.end(); it++)
  {
    auto rit = real_con.find(it.key());
    if (rit == real_con.end()) config_mismatch_error(it.key(), mip);

    if (rit->type_name() != it->type_name())
      type_mismatch_error(it.key(), rit->type_name(), it->type_name(), mip);

    rit.value() =
        it->type_name() == std::string{ "array" }
            ? life::configuration::array(
                  { it.value()[0],
                    true_object({ std::string{ rit.value()[0] }.substr(5),
                                  it.value()[0] },
                                it.value()[1]) })
            : it.value();
  }
  return real_con;
}

}   // namespace config
}   // namespace life
