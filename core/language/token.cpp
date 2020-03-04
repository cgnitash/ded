

#include <string>
#include <iostream>

#include "token.hpp"
#include "../utilities/term_colours.hpp"
#include "../utilities/utilities.hpp"

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
  if (s == "=>")
    return TokenType::tag_bind;
  if (s == "->")
    return TokenType::input_signal_bind;
  if (s == "<-")
    return TokenType::output_signal_bind;
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
  if (s.find(':') != std::string::npos)
    return TokenType::tag_name;
  return TokenType::word;
}

void errInvalidToken(Token                    token,
                     std::string              message,
                     std::vector<std::string> suggestions)
{
  auto left_padding = std::string(token.location_.column_ + 10, ' ');
  std::cout << "parse-error\n\n"
            << token.diagnostic_ << "\n"
            << left_padding << utilities::TermColours::red_fg
            << "^" << std::string(token.expr_.length() - 1, '~') << "\n"
            << left_padding<<  message;

  if (auto suggestion = rs::find_if(suggestions,
                           [word = token.expr_](auto attempt) {
                             return utilities::match(attempt, word);
                           });
     suggestion != rs::end(suggestions))
    std::cout << "\n"
              << left_padding<< "Did you mean "
              << utilities::TermColours::green_fg << *suggestion 
              << utilities::TermColours::red_fg <<  "?";

  std::cout << utilities::TermColours::reset << std::endl;
}


}   // namespace language
}   // namespace ded
