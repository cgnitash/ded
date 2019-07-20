

#include <fstream>
#include <iomanip>
#include <iostream>
#include <range/v3/all.hpp>
#include <regex>
#include <string>
#include <vector>

#include "../specs/configuration_primitive.h"
#include "../utilities/term_colours.h"
#include "../utilities/utilities.h"
#include "parser.h"

namespace ded
{
namespace language
{

// order of options matter
const std::regex Parser::valid_symbol_{
  R"~~(^(\s+|\[|\]|\{|\}|\=|\?|\$\w+|\!\w+|[\.\w]+))~~"
};

void
    Parser::parse_expression(int begin)
{
  if (begin + 2 >= static_cast<int>(source_tokens_.tokens.size()))
  {
    err_invalid_token(source_tokens_.tokens[begin],
                      "unable to parse expression syntax");
    throw ParserError{};
  }

  if (source_tokens_.tokens[begin].type_ != TokenType::word)
  {
    err_invalid_token(source_tokens_.tokens[begin],
                      "expected new variable name here");
    throw ParserError{};
  }

  if (auto prev =
          ranges::find_if(variables_,
                          [tok = source_tokens_.tokens[begin]](auto var) {
                            return var.first.expr_ == tok.expr_;
                          });
      prev != variables_.end())
  {
    err_invalid_token(source_tokens_.tokens[begin],
                      "variable re-definition not allowed");
    err_invalid_token(prev->first, "variable already defined here");
    throw ParserError{};
  }

  if (source_tokens_.tokens[begin + 1].type_ != TokenType::assignment)
    err_invalid_token(source_tokens_.tokens[begin + 1], "expected =");

  if (source_tokens_.tokens[begin + 2].type_ != TokenType::component &&
      source_tokens_.tokens[begin + 2].type_ != TokenType::variable)
  {
    err_invalid_token(source_tokens_.tokens[begin + 2],
                      "expected existing variable name or component here");
    throw ParserError{};
  }
  auto nested_block         = expand_block(begin + 2);
  nested_block.range_.first = begin + 2;
  variables_.push_back({ source_tokens_.tokens[begin], nested_block });
}

void
    Parser::err_invalid_token(Token                    tok,
                              std::string              message,
                              std::vector<std::string> suggestions)
{
  auto line             = tok.location_.first;
  auto column           = tok.location_.second;
  auto line_with_colour = source_tokens_.lines[line];
  line_with_colour.insert(column + tok.expr_.length(),
                          utilities::TermColours::reset);
  line_with_colour.insert(column, utilities::TermColours::red_fg);
  std::cout << "parse-error\nLine" << std::setw(4) << line + 1 << ":"
            << line_with_colour << utilities::TermColours::red_fg << std::endl
            << std::string(column + 9, ' ')
            << std::string(tok.expr_.length(), '~') << "\n"
            << std::string(column + 9, ' ') << utilities::TermColours::red_fg
            << "^ " << message;

  if (auto f = ranges::find_if(suggestions,
                               [word = tok.expr_](auto attempt) {
                                 return utilities::match(attempt, word);
                               });
      f != ranges::end(suggestions))
    std::cout << "\n"
              << std::string(column + 9, ' ') << "Did you mean "
              << utilities::TermColours::green_fg << *f;

  std::cout << utilities::TermColours::reset << std::endl;
}

Block
    Parser::expand_block(int begin)
{

  auto current = source_tokens_.tokens[begin].type_ == TokenType::variable
                     ? variable_block(begin)
                     : component_block(begin);

  current.range_.first = begin;

  if (begin + 1 == static_cast<int>(source_tokens_.tokens.size()) ||
      source_tokens_.tokens[begin + 1].type_ != TokenType::open_brace)
  {
    current.range_.second = begin + 1;
    return current;
  }

  return process_overrides(current, begin + 2);
}

void
    Parser::attempt_parameter_override(Block &current, int &begin)
{
  switch (source_tokens_.tokens[begin + 2].type_)
  {
    case TokenType::word:
      current.overrides_.push_back(
          { source_tokens_.tokens[begin], source_tokens_.tokens[begin + 2] });
      begin += 3;
      break;
    case TokenType::variable:
    case TokenType::component:
      current.nested_.push_back(
          { source_tokens_.tokens[begin], expand_block(begin + 2) });
      begin = current.nested_.back().second.range_.second;
      break;
    default:
      err_invalid_token(source_tokens_.tokens[begin + 2],
                        "expected override of parameter or nested spec here");
      throw ParserError{};
  }
}

/*
void
    Parser::attempt_tag_rewrite(Block &current, int &begin)
{
  if (tokens_[begin + 2].type_ != TokenType::tag_rewrite)
  {
    err_invalid_token(tokens_[begin + 2], "expected tag-rewrite here");
    throw ParserError{};
  }
  current.tag_rewrites_.push_back({ tokens_[begin], tokens_[begin + 2] });
  begin += 3;
}
*/

void
    Parser::attempt_trace(Block &current, int &begin)
{
  if (source_tokens_.tokens[begin + 1].type_ != TokenType::word)
  {
    err_invalid_token(source_tokens_.tokens[begin + 1],
                      "expected tag name here");
    throw ParserError{};
  }
  if (source_tokens_.tokens[begin + 2].type_ != TokenType::word)
  {
    err_invalid_token(source_tokens_.tokens[begin + 2],
                      "expected tag name here");
    throw ParserError{};
  }
  current.traces_.push_back(
      { source_tokens_.tokens[begin + 1], source_tokens_.tokens[begin + 2] });
  begin += 3;
}

void
    Parser::attempt_override(Block &current, int &begin)
{
  switch (source_tokens_.tokens[begin].type_)
  {
    case TokenType::word:
      attempt_parameter_override(current, begin);
      break;
    case TokenType::trace:
      attempt_trace(current, begin);
      break;
    default:
      err_invalid_token(
          source_tokens_.tokens[begin],
          "unexpected symbol: expected parameter or tag-rewrite here");
      throw ParserError{};
  }
}

Block
    Parser::process_overrides(Block current, int begin)
{

  auto scope_is_open = [&] {
    if (begin == static_cast<int>(source_tokens_.tokens.size()))
    {
      err_invalid_token(source_tokens_.tokens[current.range_.first + 1],
                        "unmatched brace");
      throw ParserError{};
    }
    return source_tokens_.tokens[begin].type_ != TokenType::close_brace;
  };

  while (scope_is_open())
  {

    if (begin + 3 >= static_cast<int>(source_tokens_.tokens.size()) ||
        (source_tokens_.tokens[begin + 1].type_ != TokenType::assignment &&
         source_tokens_.tokens[begin].type_ != TokenType::trace))
    {
      err_invalid_token(source_tokens_.tokens[begin],
                        "unable to parse override syntax");
      throw ParserError{};
    }

    attempt_override(current, begin);
  }

  current.range_.second = begin + 1;
  return current;
}

Block
    Parser::variable_block(int begin)
{
  if (auto f = ranges::find_if(variables_,
                               [tok = source_tokens_.tokens[begin]](auto var) {
                                 return var.first.expr_ == tok.expr_.substr(1);
                               });
      f == variables_.end())
  {
    err_invalid_token(source_tokens_.tokens[begin],
                      "this variable has not been defined",
                      variables_ | ranges::view::transform([](auto var) {
                        return var.first.expr_;
                      }));
    throw ParserError{};
  }
  else
  {
    return f->second;
  }
}

Block
    Parser::component_block(int begin)
{
  Block current;
  current.name_ = source_tokens_.tokens[begin].expr_;
  return current;
}

void
    Parser::parse(SourceTokens source_tokens)
{
  source_tokens_ = source_tokens;
  //  open_file(file_name);
  //  lex();

  for (auto start = 0u; start != source_tokens_.tokens.size();
       start      = variables_.back().second.range_.second)
    parse_expression(start);

  return;
}

std::optional<std::pair<int, int>>
    Parser::has_varied_parameter()
{

  auto open_variance_position = ranges::find(
      source_tokens_.tokens, TokenType::open_varied_argument, &Token::type_);
  auto close_variance_position = ranges::find(
      source_tokens_.tokens, TokenType::close_varied_argument, &Token::type_);
  if (open_variance_position == ranges::end(source_tokens_.tokens) &&
      open_variance_position == ranges::end(source_tokens_.tokens))
    return std::nullopt;

  if (open_variance_position < ranges::begin(source_tokens_.tokens) + 2 ||
      (open_variance_position - 1)->type_ != TokenType::assignment ||
      (open_variance_position - 2)->type_ != TokenType::word)
  {
    err_invalid_token(*open_variance_position,
                      "varied parameter syntax error here");
    throw ParserError{};
  }

  if (close_variance_position == ranges::end(source_tokens_.tokens))
  {
    err_invalid_token(*open_variance_position,
                      "varied parameter is not closed");
    throw ParserError{};
  }

  if (close_variance_position < open_variance_position)
  {
    err_invalid_token(*close_variance_position,
                      "unexpected ']' no varied parameter to close here");
    throw ParserError{};
  }

  if (auto f = ranges::find(open_variance_position + 1,
                            close_variance_position,
                            TokenType::open_varied_argument,
                            &Token::type_);
      f != close_variance_position)
  {
    err_invalid_token(*f, "varied parameters cannot be nested");
    throw ParserError{};
  }

  return std::make_pair(
      open_variance_position - ranges::begin(source_tokens_.tokens),
      close_variance_position - ranges::begin(source_tokens_.tokens));
}

std::vector<Parser>
    Parser::vary_parameter()
{
  //std::vector<Parser> res;
  auto                pos = has_varied_parameter();
  if (!pos)
  {
    parse(source_tokens_);
    return { *this };
  }

  std::vector<Token> subs;
  ranges::copy(ranges::begin(source_tokens_.tokens) + pos->first + 1,
               ranges::begin(source_tokens_.tokens) + pos->second,
               ranges::back_inserter(subs));

  source_tokens_.tokens.erase(ranges::begin(source_tokens_.tokens) + pos->first,
                              ranges::begin(source_tokens_.tokens) +
                                  pos->second + 1);

  return subs | ranges::view::transform([&](auto token) {
           auto   temp = source_tokens_;
           temp.tokens.insert(ranges::begin(temp.tokens) + pos->first, token);
           Parser p{*this};
           p.update_source_tokens(temp);
           p.update_labels(
               { (ranges::begin(temp.tokens) + pos->first - 2)->expr_,
                 token.expr_ });
           return p;
         });
}

// debug only
void
    Parser::print(Block b)
{
  std::cout << b.name_ << " [" << b.range_.first << "," << b.range_.second
            << "]\n";
  for (auto [name, value] : b.overrides_)
    std::cout << name.expr_ << ":" << value.expr_ << "\n";
  for (auto [name, value] : b.nested_)
  {
    std::cout << name.expr_ << "->\n";
    print(value);
  }
}
}   // namespace language
}   // namespace ded
