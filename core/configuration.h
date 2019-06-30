
#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <range/v3/all.hpp>
#include <string>
#include <vector>

#include "specs/entity_spec.h"
#include "specs/environment_spec.h"
#include "specs/population_spec.h"

#include "utilities/term_colours.h"

namespace ded
{

namespace specs
{
struct SpecError
{
};
}

extern std::string global_path;

extern std::map<std::string, specs::EntitySpec>      all_entity_specs;
extern std::map<std::string, specs::EnvironmentSpec> all_environment_specs;
extern std::map<std::string, specs::PopulationSpec>  all_population_specs;

// component_builder will provide full specializations
template <typename T>
std::string auto_class_name_as_string() = delete;

namespace config_manager
{

std::string type_of_block(std::string name);
void        show_config(std::ostream &out, std::string name);
void        list_all_configs(std::ostream &out);
void        save_all_configs();


}   // namespace config_manager
}   // namespace ded
/*
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
    check_entity_correct()
{

    for (auto &group : { "parameters", "input-tags", "output-tags" })
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
    }
  return;
}

inline void
    check_environment_correct()
{

    for (auto &group :
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
    }
  return;
}

inline void
    check_population_correct()
{


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
    }
  return;
}

inline void
    check_all_configs_correct()
{
  // no-op ??
  // check_entity_correct();
  // check_environment_correct();
  // check_population_correct();

    for (auto &[type_name, config] : life::all_configs)
    {
      if (type_name.first == "environment")
        check_environment_correct(type_name, config);
      if (type_name.first == "entity") check_entity_correct(type_name, config);
      if (type_name.first == "population")
        check_population_correct(type_name, config);
    }
}
*/
