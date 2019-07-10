
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
}   // namespace specs

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

namespace experiments
{

std::vector<std::string> open_file(std::string);

std::vector<language::Token> lex(std::vector<std::string>);

std::vector<language::Parser> explode_all_tokens(language::Parser);

std::pair<specs::PopulationSpec, specs::EnvironmentSpec>
    parse_simulation(language::Parser);

std::vector<std::pair<specs::PopulationSpec, specs::EnvironmentSpec>>
    parse_all_simulations(std::string);
}
}   // namespace ded
/*


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
