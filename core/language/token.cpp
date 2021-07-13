

#include <iostream>
#include <string>

#include "../utilities/term_colours.hpp"
#include "../utilities/utilities.hpp"
#include "token.hpp"

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
  if (s.length() > 1 && s.substr(0, 2) == "__")
    return TokenType::keyword;
  return TokenType::word;
}

void
    errInvalidToken_detail_(bool                     is_error,
                            Token                    token,
                            std::string              message,
                            std::vector<std::string> suggestions)
{
  auto source_location = token.from_file_ + ":" +
                         std::to_string(token.location_.line_ + 1) + ":" +
                         std::to_string(token.location_.column_ + 1) + " ";
  auto left_padding =
      std::string(source_location.length() + token.location_.column_, ' ');

  message = "^" + std::string(token.expr_.length() - 1, '~') +
            (is_error ? "\nError: " : "\nWarning: ") + message;

  std::cout << utilities::TermColours::cyan_fg << source_location
            << utilities::TermColours::reset << token.diagnostic_;

  for (auto line : ra::split(message, '\n'))
    std::cout << "\n"
              << (is_error ? utilities::TermColours::red_fg
                           : utilities::TermColours::magenta_fg)
              << left_padding << line;

  if (auto suggestion = rs::find_if(suggestions,
                                    [word = token.expr_](auto attempt) {
                                      return utilities::match(attempt, word);
                                    });
      suggestion != rs::end(suggestions))
    std::cout << "\n"
              << left_padding << "Did you mean "
              << utilities::TermColours::green_fg << *suggestion
              << utilities::TermColours::red_fg << "?";

  std::cout << utilities::TermColours::reset << std::endl;
}

void
    errInvalidToken(Token                    token,
                    std::string              message,
                    std::vector<std::string> suggestions)
{
  errInvalidToken_detail_(true, token, message, suggestions);
}
void
    errWarningToken(Token                    token,
                    std::string              message,
                    std::vector<std::string> suggestions)
{
  errInvalidToken_detail_(false, token, message, suggestions);
}
}   // namespace language
}   // namespace ded
