

#include "token.h"
#include <string>

namespace ded
{
namespace language
{

TokenType
    parseTokenType(std::string s)
{
  if (s == "[")
    return TokenType::open_varied_argument;
  if (s == "]")
    return TokenType::close_varied_argument;
  if (s == "<")
    return TokenType::open_nested_block_vector;
  if (s == ">")
    return TokenType::close_nested_block_vector;
  if (s == "{")
    return TokenType::open_brace;
  if (s == "}")
    return TokenType::close_brace;
  if (s == "=")
    return TokenType::assignment;
  if (s == "?")
    return TokenType::trace;
  if (s[0] == '$')
    return TokenType::component;
  if (s[0] == '!')
    return TokenType::variable;
  if (s[0] == '^')
    return TokenType::tracked_word;
  return TokenType::word;
}
}   // namespace language
}   // namespace ded
