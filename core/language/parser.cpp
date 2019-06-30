

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
  R"~~(^(\s+|\{|\}|\=|\?|\$\w+|\!\w+|[\.\w]+))~~"
};

void
    Parser::open_file(std::string file_name)
{

  std::ifstream file(file_name);
  if (!file.is_open())
  {
    std::cout << "error: " << file_name << " not found\n";
    throw ParserError{};
  }

  std::string line;
  while (std::getline(file, line))
  {
    std::string::size_type f;
    while ((f = line.find('\t')) != std::string::npos)
      line.replace(f, 1, "    ");
    lines_.push_back(line);
  }
}

void
    Parser::lex()
{
  for (auto [line_number, line] : ranges::view::enumerate(lines_))
  {
    line.erase(ranges::find(line, '#'), line.end());

    auto i = line.cbegin();
    for (std::smatch m; i != line.cend() &&
                        std::regex_search(i, line.cend(), m, valid_symbol_);
         i += m.str().length())
      if (!ranges::all_of(m.str(), ::isspace))
        tokens_.push_back(Token{ parse_token_type(m.str()),
                                 m.str(),
                                 { line_number, i - line.cbegin() } });

    if (i != line.cend())
      err_unknown_symbol({ line_number, i - line.cbegin() });
  }
}

void
    Parser::err_unknown_symbol(std::pair<int, int> location)
{

  std::cout << "parse-error:" << location.first + 1 << ":"
            << location.second + 1 << "\n"
            << lines_[location.first] << "\n"
            << std::string(location.second, ' ')
            << utilities::TermColours::red_fg << "^ unknown symbol\n"
            << utilities::TermColours::reset;
  throw ParserError{};
}

void
    Parser::parse_expression(int begin)
{
  if (begin + 2 >= static_cast<int>(tokens_.size()))
  {
    err_invalid_token(tokens_[begin], "unable to parse expression syntax");
    throw ParserError{};
  }

  if (tokens_[begin].type_ != TokenType::word)
  {
    err_invalid_token(tokens_[begin], "expected new variable name here");
    throw ParserError{};
  }

  if (auto prev = ranges::find_if(variables_,
                                  [tok = tokens_[begin]](auto var) {
                                    return var.first.expr_ == tok.expr_;
                                  });
      prev != variables_.end())
  {
    err_invalid_token(tokens_[begin], "variable re-definition not allowed");
    err_invalid_token(prev->first, "variable already defined here");
    throw ParserError{};
  }

  if (tokens_[begin + 1].type_ != TokenType::assignment)
    err_invalid_token(tokens_[begin + 1], "expected =");

  if (tokens_[begin + 2].type_ != TokenType::component &&
      tokens_[begin + 2].type_ != TokenType::variable)
  {
    err_invalid_token(tokens_[begin + 2],
                      "expected existing variable name or component here");
    throw ParserError{};
  }
  auto nested_block         = expand_block(begin + 2);
  nested_block.range_.first = begin + 2;
  variables_.push_back({ tokens_[begin], nested_block });
}

void
    Parser::err_invalid_token(Token                    tok,
                              std::string              message,
                              std::vector<std::string> suggestions)
{
  auto line             = tok.location_.first;
  auto column           = tok.location_.second;
  auto line_with_colour = lines_[line];
  line_with_colour.insert(column + tok.expr_.length(),
                          utilities::TermColours::reset);
  line_with_colour.insert(column, utilities::TermColours::red_fg);
  std::cout << "parse-error\nLine" << std::setw(4) << line + 1 << ":"
            << line_with_colour << utilities::TermColours::red_fg << std::endl
            << std::string(column + 9, ' ')
            << std::string(tok.expr_.length(), '~') << "\n"
            << std::string(column + 9, ' ') << utilities::TermColours::blue_fg
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

  auto current = tokens_[begin].type_ == TokenType::variable
                     ? variable_block(begin)
                     : component_block(begin);

  current.range_.first = begin;

  if (begin + 1 == static_cast<int>(tokens_.size()) ||
      tokens_[begin + 1].type_ != TokenType::open_brace)
  {
    current.range_.second = begin + 1;
    return current;
  }

  return process_overrides(current, begin + 2);
}

void
    Parser::attempt_parameter_override(Block &current, int &begin)
{
  switch (tokens_[begin + 2].type_)
  {
    case TokenType::word:
      current.overrides_.push_back({ tokens_[begin], tokens_[begin + 2] });
      begin += 3;
      break;
    case TokenType::variable:
    case TokenType::component:
      current.nested_.push_back({ tokens_[begin], expand_block(begin + 2) });
      begin = current.nested_.back().second.range_.second;
      break;
    default:
      err_invalid_token(tokens_[begin + 2],
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
  if (tokens_[begin + 1].type_ != TokenType::word)
  {
    err_invalid_token(tokens_[begin + 1], "expected tag name here");
    throw ParserError{};
  }
  if (tokens_[begin + 2].type_ != TokenType::word)
  {
    err_invalid_token(tokens_[begin + 2], "expected tag name here");
    throw ParserError{};
  }
  current.traces_.push_back({ tokens_[begin + 1], tokens_[begin + 2] });
  begin += 3;
}

void
    Parser::attempt_override(Block &current, int &begin)
{
  switch (tokens_[begin].type_)
  {
    case TokenType::word:
      attempt_parameter_override(current, begin);
      break;
    case TokenType::trace:
      attempt_trace(current, begin);
      break;
    default:
      err_invalid_token(
          tokens_[begin],
          "unexpected symbol: expected parameter or tag-rewrite here");
      throw ParserError{};
  }
}

Block
    Parser::process_overrides(Block current, int begin)
{

  auto scope_is_open = [&] {
    if (begin == static_cast<int>(tokens_.size()))
    {
      err_invalid_token(tokens_[current.range_.first + 1], "unmatched brace");
      throw ParserError{};
    }
    return tokens_[begin].type_ != TokenType::close_brace;
  };

  while (scope_is_open())
  {

    if (begin + 3 >= static_cast<int>(tokens_.size()) ||
        (tokens_[begin + 1].type_ != TokenType::assignment &&
         tokens_[begin].type_ != TokenType::trace))
    {
      err_invalid_token(tokens_[begin], "unable to parse override syntax");
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
                               [tok = tokens_[begin]](auto var) {
                                 return var.first.expr_ == tok.expr_.substr(1);
                               });
      f == variables_.end())
  {
    err_invalid_token(tokens_[begin],
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
  current.name_ = tokens_[begin].expr_;
  return current;
}

void
    Parser::parse(std::string file_name)
{

  open_file(file_name);
  lex();
  for (auto start = 0u; start != tokens_.size();
       start      = variables_.back().second.range_.second)
    parse_expression(start);

  return;
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
