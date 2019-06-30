

#include "token.h"
#include <string>

namespace ded
{
namespace language
{

TokenType
    parse_token_type(std::string s)
{
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
  return TokenType::word;
}
}   // namespace ded
}   // namespace ded
