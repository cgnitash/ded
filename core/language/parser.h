
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

struct SourceTokens
{
  std::string                          file_name;
  std::vector<std::string>             lines;
  std::vector<Token>                   tokens;
};

class Parser
{

  SourceTokens source_tokens_;

  std::vector<std::pair<Token, Block>> variables_;


  void err_invalid_token(Token, std::string, std::vector<std::string> = {});
  /*
  void open_file(std::string);

  void lex();
  void err_unknown_symbol(std::pair<int, int>);
	*/

  void parse_expression(int);

  Block expand_block(int);

  Block component_block(int);
  Block variable_block(int);

  Block process_overrides(Block, int);

  void attempt_override(Block &, int &);
  void attempt_parameter_override(Block &, int &);
  void attempt_trace(Block &, int &);


public:
  const static std::regex valid_symbol_;
  //void parse(std::string);
  Parser(SourceTokens s) { source_tokens_ = s; }
  Parser() = default;

  auto
      lines() const
  {
    return source_tokens_.lines;
  }
  auto
      file_name() const
  {
    return source_tokens_.file_name;
  }
  auto
      source_tokens() const
  {
    return source_tokens_;
  }
  void parse(SourceTokens);
  std::vector<std::pair<Token, Block>>
      variables() const
  {
    return variables_;
  }
  std::vector<Parser> vary_parameter();
  std::optional<std::pair<int, int>> has_varied_parameter();
  void print(Block b);

  friend class specs::EnvironmentSpec;
  friend class specs::EntitySpec;
  friend class specs::PopulationSpec;
};
}   // namespace language
}   // namespace ded
