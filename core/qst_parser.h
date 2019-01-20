
#pragma once

#include <fstream>
#include <iostream>
#include <range/v3/all.hpp>
#include <regex>
#include <string>
#include <utility>
#include <vector>

class qst_parser {

  std::regex comments{R"~~(#.*$)~~"};
  std::regex close_brace{R"~~(^\s*}\s*$)~~"};
  std::regex spurious_commas{R"~~(,(]|}))~~"};
  std::regex new_variable{
      R"~~(^\s*([-\w\d]+)\s*=\s*\$([-\w\d]+)\s*(\{)?\s*$)~~"};
  std::regex new_refactored_variable{
      R"~~(^\s*([-\w\d]+)\s*=\s*!([-\w\d]+)\s*$)~~"};
  std::regex new_varied_variable{
      R"~~(^\s*([-\w\d]+)\s*=\s*\[([^\]]+)\]\s*$)~~"};
  std::regex nested_parameter{
      R"~~(^\s*vary\s+([-\w\d]+)\s*=\s*\$([-\w\d]+)\s*(\{)?\s*$)~~"};
  std::regex nested_refactored_parameter{
      R"~~(^\s*vary\s+([-\w\d]+)\s*=\s*!([-\w\d]+)\s*$)~~"};
  std::regex nested_varied_parameter{
      R"~~(^\s*vary\s+([-\w\d]+)\s*=\s*\[([^\]]+)\]\s*$)~~"};
  std::regex parameter{R"~~(^\s*vary\s*([-\w\d]+)\s*=\s*([-\.\w\d]+)\s*$)~~"};
  std::regex pre_tag{R"~~(^\s*pre\s*([-\w\d]+)\s*=\s*([-\w\d]+)\s*$)~~"};
  std::regex post_tag{R"~~(^\s*pos\s*([-\w\d]+)\s*=\s*([-\w\d]+)\s*$)~~"};
  std::regex in_signal_tag{R"~~(^\s*ist\s*([-\w\d]+)\s*=\s*([-\w\d]+)\s*$)~~"};
  std::regex out_signal_tag{R"~~(^\s*ost\s*([-\w\d]+)\s*=\s*([-\w\d]+)\s*$)~~"};

public:
  std::vector<std::string> expand_layout(std::string,
                                         std::vector<std::vector<std::string>>);
  std::vector<std::tuple<std::string, std::string, std::string>>
      parse_qst(std::string);
};

