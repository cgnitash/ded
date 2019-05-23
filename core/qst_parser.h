
#pragma once

#include <fstream>
#include <iostream>
#include <range/v3/all.hpp>
#include <regex>
#include <string>
#include <utility>
#include <vector>

class qst_parser {

  std::regex comments{ R"~~(#.*$)~~" };
  std::regex spurious_tabs{ R"~~(\t)~~" };

  std::regex new_variable{
    R"~~(^\s*([-\w\d]+)\s*=\s*\$([-\w\d]+)\s*(\{)?\s*$)~~"
  };
  std::regex new_refactored_variable{
    R"~~(^\s*([-\w\d]+)\s*=\s*!([-\w\d]+)\s*$)~~"
  };
  std::regex new_varied_variable{
    R"~~(^\s*([-\w\d]+)\s*=\s*\[([^\]]+)\]\s*$)~~"
  };
  std::regex nested_parameter{
    R"~~(^\s*vary\s+([-\w\d]+)\s*=\s*\$([-\w\d]+)\s*(\{)?\s*$)~~"
  };
  std::regex nested_refactored_parameter{
    R"~~(^\s*vary\s+([-\w\d]+)\s*=\s*!([-\w\d]+)\s*$)~~"
  };
  std::regex nested_varied_parameter{
    R"~~(^\s*vary\s+([-\w\d]+)\s*=\s*\[([^\]]+)\]\s*$)~~"
  };
  std::regex parameter{ R"~~(^\s*vary\s*([-\w\d]+)\s*=\s*(.+)\s*$)~~" };
  std::regex pre_tag{ R"~~(^\s*([-\w\d]+)\s*<-\s*([-\w\d]+)\s*$)~~" };
  std::regex post_tag{ R"~~(^\s*([-\w\d]+)\s*->\s*([-\w\d]+)\s*$)~~" };
  std::regex in_signal_tag{
    R"~~(^\s*ist\s*([-\w\d]+)\s*=\s*([-\w\d]+)\s*$)~~"
  };
  std::regex out_signal_tag{
    R"~~(^\s*ost\s*([-\w\d]+)\s*=\s*([-\w\d]+)\s*$)~~"
  };

  std::regex close_brace{ R"~~(^\s*}\s*$)~~" };
  std::regex spurious_commas{ R"~~(,(]|}))~~" };

  std::vector<std::tuple<std::string, std::string, std::string>> all_exps;
  std::map<std::string, std::string>                             all_variables;
  struct component_spec
  {
    std::string variable_or_comp_name, comp, params, pres, posts, in_sigs,
        out_sigs;
  };
  std::vector<component_spec>           component_stack;
  std::vector<std::vector<std::string>> varied;
  std::vector<std::vector<std::string>> varied_labels;
  long                                  line_num{ 1 };
  bool                                  in_nested_scope{ false };

  // methods
  void parse_new_variable(std::smatch);
  void parse_new_refactored_variable(std::smatch);
  void parse_new_varied_variable(std::smatch);
  void parse_nested_parameter(std::smatch);
  void parse_nested_refactored_parameter(std::smatch);
  void parse_nested_varied_parameter(std::smatch);
  void parse_closed_brace();
  void cleanup();
  void check_no_redefinition(std::string);
  void check_global_scope();

public:
  qst_parser() = default;
  std::vector<std::string> expand_layout(std::string,
                                         std::vector<std::vector<std::string>>);
  std::vector<std::tuple<std::string, std::string, std::string>>
      parse_qst(std::string);
};
