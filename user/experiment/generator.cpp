
#include "generator.h"

#include <algorithm>
#include <experimental/filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

void generator::check_jar_types(life::configuration jar,
                                std::string         patch_path,
                                std::string         required_type)
{

  for (auto &j : jar)
    if (j.type_name() != required_type)
    {
      std::cout << "Error: varied parameter \033[33m" << patch_path
                << "\033[0m has type \033[31m'" << required_type
                << "'\033[0m but varied parameter \033[34m> " << j.dump()
                << "\033[0m has type \033[31m'" << j.type_name()
                << "'\033[0m\n";
      std::exit(1);
    }
}

void generator::check_parameter_override_error(life::configuration con,
                                               std::string         patch_path)
{
  try
  {
    con.at(life::configuration::json_pointer{ patch_path });
  } catch (...)
  {

    std::cout << "Error: some user parameter problem with \033[31m'"
              << patch_path << "'\033[0m\n";
    std::exit(1);
  }
}

// bool generator::does_not_override(life::configuration con, std::string comm)
// {

life::configuration generator::true_user_experiment(std::string file_name)
{
  life::configuration con;
  std::ifstream       ifs(file_name);
  if (!ifs.is_open())
  {
    std::cout << "Error: experiment file \"" << file_name
              << "\" does not exist\n";
    std::exit(1);
  }
  ifs >> con;

  return life::configuration::array(
      { con[0], life::config::true_object({ "experiment", con[0] }, con[1]) });
}

void generator::run()
{

  std::ifstream ifs(file_);
  if (!ifs.is_open())
  {
    std::cout << "Error: generator script \"" << file_ << "\" does not exist\n";
    std::exit(1);
  }
  std::vector<std::string> lines;
  std::string              line;
  while (std::getline(ifs, line)) lines.push_back(line);

  std::map<size_t, life::configuration> m;
  for (const auto &con_set : lines | clean_lines() | by_vary() |
                                 expand_true_experiment() |
                                 ranges::action::join)
    for (auto &c : con_set)
    {
      m[hash_experiment(c.dump())] = c;
      std::cout << hash_experiment(c.dump()) << std::endl;
    }
  auto dir = life::global_path + "exec/";
  if (!std::experimental::filesystem::exists(dir))
    std::experimental::filesystem::create_directory(dir);

  for (auto &[hash, con] : m)
  {
    auto ed = dir + std::to_string(hash);
    std::experimental::filesystem::create_directory(ed);
    std::ofstream ofs(ed + "/this.json");
    ofs << con.dump(4);
  }
}
