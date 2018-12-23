
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

std::vector<life::configuration> generator::parse() {

  std::ifstream ifs(file_);
  if (!ifs.is_open()) {
    std::cout << "Error: generator script \"" << file_
              << "\" does not exist\n";
    std::exit(1);
  }
  std::vector<std::string> lines;
  std::string line;
  while(std::getline(ifs,line)) 
	  lines.push_back(line);

  for (auto l : lines | clean_lines() | by_vary() )//| expand_experiment())
    std::cout << l << std::endl;

  return {};


}
void generator::run() {

  auto con_set = parse();

  auto dir = life::global_path + "data/";
  if (!std::experimental::filesystem::exists(dir)) 
   std::experimental::filesystem::create_directory(dir);

  //std::ofstream ofs(dir + "attempt.txt"); 
  //
}

