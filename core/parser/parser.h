
#pragma once

#include "token.h"
#include <regex>
#include <string>
#include <vector>

namespace life {

class parser {

  std::vector<std::string>             lines_;
  std::vector<token>                   tokens_;
  std::vector<std::pair<token, block>> variables_;

  const static std::regex valid_symbol_;

  void open_file(std::string);

  void lex();
  void err_unknown_symbol(std::pair<int, int>);

  void parse_expression(int);

  void err_invalid_token(token, std::string);

  block expand_block(int);

  block component_block(int);
  block variable_block(int);

  block process_overrides(block, int);

  void attempt_override(block &, int &);
  void attempt_parameter_override(block &, int &);
  //void attempt_tag_rewrite(block &, int &);
  void attempt_trace(block &, int &);

public:
  void                                 parse(std::string);
  std::vector<std::pair<token, block>> variables() const { return variables_; }
  void                                 print(block b);
  friend class environment_spec;
  friend class entity_spec;
  friend class population_spec;
};
}   // namespace life

