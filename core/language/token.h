

#pragma once

#include <optional>
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
  std::string         diagnostic_;
  std::string         refers_;
};

/*
struct NameToken
{
  std::string line;
  std::pair<int, int> pos;
};
*/

struct Block
{
  Token                                             name_token_;
  std::string                                       name_;
  std::pair<int, int>                               range_;
  std::vector<std::pair<Token, Token>>              overrides_;
  std::vector<std::pair<Token, Token>>              traces_;
  std::vector<std::pair<Token, Block>>              nested_;
  std::vector<std::pair<Token, std::vector<Block>>> nested_vector_;
};

}   // namespace language
}   // namespace ded
