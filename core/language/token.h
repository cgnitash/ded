

#pragma once

#include <string>
#include <vector>
#include <optional>

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
  tracked_word,
  word,
  component,
  variable,
  open_varied_argument,
  close_varied_argument,
  open_nested_block_vector,
  close_nested_block_vector
};

TokenType parseTokenType(std::string);

struct Token
{
  TokenType           type_;
  std::string         expr_;
  std::pair<int, int> location_;
  std::string refers_;
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
