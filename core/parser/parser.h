
#pragma once

#include "../specs/entity_spec.h"
#include "../specs/environment_spec.h"
#include "../specs/population_spec.h"
#include "token.h"
#include <string>
#include <vector>

namespace life {

class parser {

  std::vector<std::string>                   lines_;
  std::vector<token>                         tokens_;
  //std::vector<std::pair<int, int>>           brace_pair_locations_;
  std::vector<std::pair<token, block>> variables_;

  //std::string current_block_name_;
  //int         current_block_count_ = 0;
  /*
  std::map<std::string,environment_block> environment_variables_;
  std::map<std::string,population_block> population_variables_;
  std::map<std::string,entity_block> entity_variables_;
  */
  const static std::regex valid_symbol_;

  /*
  void err_dangling_brace(token);
  void err_unmatched_brace(token);
  void err_expected_word_syntax(token);
  void err_expected_assignment_syntax(token);
  void err_expected_component_syntax(token);
  void err_expected_block_scope_or_name(token);
  void  err_trailing_syntax(token);

  void err_invalid_variable_syntax(token);
  void err_invalid_expression_syntax(token,token);
  void err_variable_redefinition(token, token);
*/
  void open_file(std::string);

  void lex();
  void err_unknown_symbol(std::pair<int, int>);

  //void match_braces();
  //void check_syntax();
  //void flatten_blocks();
  //void extract_block(std::vector<token>);
  void parse_expression(int);

  void err_invalid_token(token, std::string);

  block expand_block(int);

  block component_block(int);

  block process_overrides(block, int);


void print(block b) ;
public:
  std::pair<population_spec, environment_spec> parse(std::string);
};
}   // namespace life

