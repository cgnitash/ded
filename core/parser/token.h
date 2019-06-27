

#pragma once

#include <string>
#include <vector>

namespace life
{

struct parser_error
{
};

enum class token_type
{
  open_brace,
  close_brace,
  assignment,
  trace,
  word,
  component,
  variable
};

token_type parse_token_type(std::string);

struct token
{
  token_type          type_;
  std::string         expr_;
  std::pair<int, int> location_;
};

struct block
{
  std::string                          name_;
  std::pair<int, int>                  range_;
  std::vector<std::pair<token, token>> overrides_;
  std::vector<std::pair<token, token>> traces_;
  std::vector<std::pair<token, block>> nested_;
};

}   // namespace life

