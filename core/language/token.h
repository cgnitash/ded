

#pragma once

#include <string>
#include <vector>

namespace ded
{
namespace language
{

struct ParserError
{
};

enum class TokenType
{
  open_brace,
  close_brace,
  assignment,
  trace,
  word,
  component,
  variable
};

TokenType parse_token_type(std::string);

struct Token
{
  TokenType          type_;
  std::string         expr_;
  std::pair<int, int> location_;
};

struct Block
{
  std::string                          name_;
  std::pair<int, int>                  range_;
  std::vector<std::pair<Token, Token>> overrides_;
  std::vector<std::pair<Token, Token>> traces_;
  std::vector<std::pair<Token, Block>> nested_;
};

}   // namespace language
}   // namespace ded

