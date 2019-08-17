

#include <fstream>
#include <iomanip>
#include <iostream>
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
  R"~~(^(\s+|\[|\]|\{|\}|\=|\?|\$\w+|\!\w+|\^[-\w]+|"[^"]*"|[\w\.]+))~~"
};

void
    Parser::parseExpression(int begin)
{
  if (begin + 2 >= static_cast<int>(source_tokens_.tokens.size()))
  {
    errInvalidToken(source_tokens_.tokens[begin],
                      "unable to parse expression syntax");
    throw ParserError{};
  }

  if (source_tokens_.tokens[begin].type_ != TokenType::word)
  {
    errInvalidToken(source_tokens_.tokens[begin],
                      "expected new variable name here");
    throw ParserError{};
  }

  if (auto prev =
          rs::find_if(variables_,
                          [tok = source_tokens_.tokens[begin]](auto var) {
                            return var.first.expr_ == tok.expr_;
                          });
      prev != variables_.end())
  {
    errInvalidToken(source_tokens_.tokens[begin],
                      "variable re-definition not allowed");
    errInvalidToken(prev->first, "variable already defined here");
    throw ParserError{};
  }

  if (source_tokens_.tokens[begin + 1].type_ != TokenType::assignment)
    errInvalidToken(source_tokens_.tokens[begin + 1], "expected =");

  if (source_tokens_.tokens[begin + 2].type_ != TokenType::component &&
      source_tokens_.tokens[begin + 2].type_ != TokenType::variable)
  {
    errInvalidToken(source_tokens_.tokens[begin + 2],
                      "expected existing variable name or component here");
    throw ParserError{};
  }
  auto nested_block         = expandBlock(begin + 2);
  nested_block.range_.first = begin + 2;
  variables_.push_back({ source_tokens_.tokens[begin], nested_block });
}

void
    Parser::errInvalidToken(Token                    tok,
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

  if (auto f = rs::find_if(suggestions,
                               [word = tok.expr_](auto attempt) {
                                 return utilities::match(attempt, word);
                               });
      f != rs::end(suggestions))
    std::cout << "\n"
              << std::string(column + 9, ' ') << "Did you mean "
              << utilities::TermColours::green_fg << *f;

  std::cout << utilities::TermColours::reset << std::endl;
}

Block
    Parser::expandBlock(int begin)
{

  auto current = source_tokens_.tokens[begin].type_ == TokenType::variable
                     ? variableBlock(begin)
                     : componentBlock(begin);

  current.range_.first = begin;

  if (begin + 1 == static_cast<int>(source_tokens_.tokens.size()) ||
      source_tokens_.tokens[begin + 1].type_ != TokenType::open_brace)
  {
    current.range_.second = begin + 1;
    return current;
  }

  return processOverrides(current, begin + 2);
}

Block
    Parser::processOverrides(Block current, int begin)
{

  auto scope_is_open = [&] {
    if (begin == static_cast<int>(source_tokens_.tokens.size()))
    {
      errInvalidToken(source_tokens_.tokens[current.range_.first + 1],
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
      errInvalidToken(source_tokens_.tokens[begin],
                        "unable to parse override syntax");
      throw ParserError{};
    }

    attemptOverride(current, begin);
  }

  current.range_.second = begin + 1;
  return current;
}

void
    Parser::attemptOverride(Block &current, int &begin)
{
  switch (source_tokens_.tokens[begin].type_)
  {
    case TokenType::word:
      attemptParameterOverride(current, begin);
      break;
    case TokenType::trace:
      attemptTrace(current, begin);
      break;
    default:
      errInvalidToken(
          source_tokens_.tokens[begin],
          "unexpected symbol: expected parameter or tag-rewrite? here");
      throw ParserError{};
  }
}

void
    Parser::attemptParameterOverride(Block &current, int &begin)
{
  switch (source_tokens_.tokens[begin + 2].type_)
  {
    case TokenType::word:
    case TokenType::tracked_word:
      if (auto f = rs::find_if(current.overrides_,
                                   [&](auto over) {
                                     return over.first.expr_ ==
                                            source_tokens_.tokens[begin].expr_;
                                   });
          f != rs::end(current.overrides_))
      {
        errInvalidToken(f->first, "parameters already overridden here");
        errInvalidToken(source_tokens_.tokens[begin],
                          "cannot override this parameter again");
        throw ParserError{};
      }

      current.overrides_.push_back(
          { source_tokens_.tokens[begin], source_tokens_.tokens[begin + 2] });
      begin += 3;
      break;
    case TokenType::variable:
    case TokenType::component:
      if (auto f = rs::find_if(current.nested_,
                                   [&](auto over) {
                                     return over.first.expr_ ==
                                            source_tokens_.tokens[begin].expr_;
                                   });
          f != rs::end(current.nested_))
      {
        errInvalidToken(f->first, "nested component already overridden here");
        errInvalidToken(source_tokens_.tokens[begin],
                          "cannot override this nested component again");
        throw ParserError{};
      }
      current.nested_.push_back(
          { source_tokens_.tokens[begin], expandBlock(begin + 2) });
      begin = current.nested_.back().second.range_.second;
      break;
    default:
      errInvalidToken(source_tokens_.tokens[begin + 2],
                        "expected override of parameter or nested spec here");
      throw ParserError{};
  }
}

void
    Parser::attemptTrace(Block &current, int &begin)
{
  if (source_tokens_.tokens[begin + 1].type_ != TokenType::word)
  {
    errInvalidToken(source_tokens_.tokens[begin + 1],
                      "expected tag name here");
    throw ParserError{};
  }
  if (source_tokens_.tokens[begin + 2].type_ != TokenType::word)
  {
    errInvalidToken(source_tokens_.tokens[begin + 2],
                      "expected tag name here");
    throw ParserError{};
  }
  current.traces_.push_back(
      { source_tokens_.tokens[begin + 1], source_tokens_.tokens[begin + 2] });
  begin += 3;
}

Block
    Parser::variableBlock(int begin)
{
  if (auto f = rs::find_if(variables_,
                               [tok = source_tokens_.tokens[begin]](auto var) {
                                 return var.first.expr_ == tok.expr_.substr(1);
                               });
      f == variables_.end())
  {
    errInvalidToken(source_tokens_.tokens[begin],
                      "this variable has not been defined",
                      variables_ | rv::transform([](auto var) {
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
    Parser::componentBlock(int begin)
{
  Block current;
  current.name_ = source_tokens_.tokens[begin].expr_;
  if (rs::none_of(
          config_manager::allComponentNames(),
          [&](auto comp_name) { return comp_name == current.name_.substr(1); }))
  {
    errInvalidToken(source_tokens_.tokens[begin],
                      "this is not an exisiting component",
                      config_manager::allComponentNames());
    throw ParserError{};
  }

  return current;
}

void
    Parser::parse(SourceTokens source_tokens)
{
  source_tokens_ = source_tokens;
  //  openFile(file_name);
  //  lexTokens();

  for (auto start = 0u; start != source_tokens_.tokens.size();
       start      = variables_.back().second.range_.second)
    parseExpression(start);

  return;
}

std::optional<std::pair<int, int>>
    Parser::hasVariedParameter()
{

  auto open_variance_position = rs::find(
      source_tokens_.tokens, TokenType::open_varied_argument, &Token::type_);
  auto close_variance_position = rs::find(
      source_tokens_.tokens, TokenType::close_varied_argument, &Token::type_);
  if (open_variance_position == rs::end(source_tokens_.tokens) &&
      open_variance_position == rs::end(source_tokens_.tokens))
    return std::nullopt;

  if (open_variance_position < rs::begin(source_tokens_.tokens) + 2 ||
      (open_variance_position - 1)->type_ != TokenType::assignment ||
      (open_variance_position - 2)->type_ != TokenType::word)
  {
    errInvalidToken(*open_variance_position,
                      "varied parameter syntax error here");
    throw ParserError{};
  }

  if (close_variance_position == rs::end(source_tokens_.tokens))
  {
    errInvalidToken(*open_variance_position,
                      "varied parameter is not closed");
    throw ParserError{};
  }

  if (close_variance_position < open_variance_position)
  {
    errInvalidToken(*close_variance_position,
                      "unexpected ']' no varied parameter to close here");
    throw ParserError{};
  }

  if (auto f = rs::find(open_variance_position + 1,
                            close_variance_position,
                            TokenType::open_varied_argument,
                            &Token::type_);
      f != close_variance_position)
  {
    errInvalidToken(*f, "varied parameters cannot be nested");
    throw ParserError{};
  }

  if (open_variance_position + 1 == close_variance_position)
  {
    errInvalidToken(*open_variance_position, "varying range is empty ");
    throw ParserError{};
  }

  if (open_variance_position + 2 == close_variance_position)
  {
    errInvalidToken(*(open_variance_position + 1),
                      "varying on a single value is redundant");
    throw ParserError{};
  }

  return std::make_pair(
      open_variance_position - rs::begin(source_tokens_.tokens),
      close_variance_position - rs::begin(source_tokens_.tokens));
}

std::vector<Parser>
    Parser::varyParameter()
{
  auto pos = hasVariedParameter();
  if (!pos)
  {
    parse(source_tokens_);
    return { *this };
  }

  std::vector<Token> subs;
  rs::copy(rs::begin(source_tokens_.tokens) + pos->first + 1,
               rs::begin(source_tokens_.tokens) + pos->second,
               rs::back_inserter(subs));

  source_tokens_.tokens.erase(rs::begin(source_tokens_.tokens) + pos->first,
                              rs::begin(source_tokens_.tokens) +
                                  pos->second + 1);

  return subs | rv::transform([&](auto token) {
           auto temp = source_tokens_;
           temp.tokens.insert(rs::begin(temp.tokens) + pos->first, token);
           Parser p{ *this };
           p.updateSourceTokens(temp);
           p.updateLabels(
               { (rs::begin(temp.tokens) + pos->first - 2)->expr_,
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
void
    Parser::resolveTrackedWords()
{

  for (auto &var : variables_)
  {
    replaceTokenWord(var.second);
  }
}

void
    Parser::replaceTokenWord(Block &block)
{
  for (auto &over : block.overrides_)
  {
    if (!over.second.refers_.empty())
      over.second.expr_ = lookUpTokenWord(over.second);
  }
  {
    for (auto &nested : block.nested_)
      replaceTokenWord(nested.second);
  }
}

std::string
    Parser::lookUpTokenWord(Token token)
{
  auto                     refers = token.refers_.substr(1);
  std::vector<std::string> pats   = refers | rv::split('-');

  auto f = rs::find_if(
      variables_, [&](auto var) { return var.first.expr_ == pats[0]; });
  if (f == rs::end(variables_))
  {
    errInvalidToken(token, "tracked path " + refers + " is not valid");
    throw ParserError{};
  }

  auto b = f->second;
  for (auto i = 1u; i < pats.size() - 1; i++)
  {
    auto nested = pats[i];

    auto nb = rs::find_if(
        b.nested_, [&](auto var) { return var.first.expr_ == nested; });
    if (nb == rs::end(b.nested_))
    {
      errInvalidToken(token,
                        "tracked path " + refers + " is not valid: '" + nested +
                            "' is not a nested component");
      throw ParserError{};
    }
    b = nb->second;
  }

  auto param = pats.back();

  auto par = rs::find_if(
      b.overrides_, [&](auto var) { return var.first.expr_ == param; });
  if (par == rs::end(b.overrides_))
  {
    errInvalidToken(token,
                      "tracked path " + refers + " is not valid: '" + param +
                          "' is not a parameter");
    throw ParserError{};
  }
  return par->second.expr_;
}
}   // namespace language
}   // namespace ded
