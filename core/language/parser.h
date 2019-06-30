
#pragma once

#include "token.h"
#include <regex>
#include <string>
#include <vector>

namespace ded
{

// forward declaration to provide friendship
namespace specs
{
class EntitySpec;
class EnvironmentSpec;
class PopulationSpec;
}   // namespace specs

namespace language
{

class Parser
{

  std::vector<std::string>             lines_;
  std::vector<Token>                   tokens_;
  std::vector<std::pair<Token, Block>> variables_;

  const static std::regex valid_symbol_;

  void open_file(std::string);

  void lex();
  void err_unknown_symbol(std::pair<int, int>);

  void parse_expression(int);

  void err_invalid_token(Token, std::string, std::vector<std::string> = {});

  Block expand_block(int);

  Block component_block(int);
  Block variable_block(int);

  Block process_overrides(Block, int);

  void attempt_override(Block &, int &);
  void attempt_parameter_override(Block &, int &);
  void attempt_trace(Block &, int &);

public:
  void parse(std::string);
  std::vector<std::pair<Token, Block>>
      variables() const
  {
    return variables_;
  }
  void print(Block b);

  friend class specs::EnvironmentSpec;
  friend class specs::EntitySpec;
  friend class specs::PopulationSpec;
};
}   // namespace language
}   // namespace ded
