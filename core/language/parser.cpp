

#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "../specs/configuration_primitive.hpp"
#include "../utilities/term_colours.hpp"
#include "../utilities/utilities.hpp"
#include "parser.hpp"

namespace ded
{
namespace language
{

void
    Parser::parseExpression(int begin)
{
  auto const tokens = lexer_.getTokens();

  if (begin + 3 >= static_cast<int>(tokens.size()))
  {
    errInvalidToken(tokens[begin], "unable to parse expression syntax");
    throw ParserError{};
  }

  if (tokens[begin].type_ != TokenType::keyword ||
      !rs::contains(
          std::vector<std::string>{
              "process", "substrate", "encoding", "population", "converter" },
          tokens[begin].expr_.substr(2)))
  {
    errInvalidToken(tokens[begin], "expected keyword for new definition here");
    throw ParserError{};
  }

  if (tokens[begin + 1].type_ != TokenType::word)
  {
    errInvalidToken(tokens[begin + 1], "expected new variable name here");
    throw ParserError{};
  }

  if (auto prev = rs::find_if(variables_,
                              [tok = tokens[begin + 1]](auto var) {
                                return var.user_name_.expr_ == tok.expr_;
                              });
      prev != variables_.end())
  {
    errInvalidToken(tokens[begin + 1], "variable re-definition not allowed");
    errInvalidToken(prev->user_name_, "variable already defined here");
    throw ParserError{};
  }

  if (tokens[begin + 2].type_ != TokenType::assignment)
    errInvalidToken(tokens[begin + 1], "expected =");

  if (tokens[begin + 3].type_ != TokenType::component &&
      tokens[begin + 3].type_ != TokenType::variable)
  {
    errInvalidToken(tokens[begin + 3],
                    "expected existing variable name or component here",
                    variables_ | rv::transform([](auto var) {
                      return var.user_name_.expr_;
                    }) | rs::to<std::vector<std::string>>);
    throw ParserError{};
  }
  auto nested_block          = expandBlock(begin + 3);
  nested_block.range_.begin_ = begin + 3;
  variables_.push_back({ tokens[begin], tokens[begin + 1], nested_block });
}

Block
    Parser::expandBlock(int begin)
{
  auto const tokens = lexer_.getTokens();

  auto current = tokens[begin].type_ == TokenType::variable
                     ? variableBlock(begin)
                     : componentBlock(begin);

  current.range_.begin_ = begin;

  if (begin + 1 == static_cast<int>(tokens.size()) ||
      tokens[begin + 1].type_ != TokenType::open_brace)
  {
    current.range_.end_ = begin + 1;
    return current;
  }

  return processOverrides(current, begin + 2);
}

Block
    Parser::processOverrides(Block current, int begin)
{
  auto const tokens = lexer_.getTokens();

  auto scope_is_open = [&] {
    if (begin == static_cast<int>(tokens.size()))
    {
      errInvalidToken(tokens[current.range_.begin_ + 1], "unmatched brace");
      throw ParserError{};
    }
    return tokens[begin].type_ != TokenType::close_brace;
  };

  while (scope_is_open())
  {

    if (begin + 3 >= static_cast<int>(tokens.size()) ||
        (tokens[begin + 1].type_ != TokenType::assignment &&
         tokens[begin + 1].type_ != TokenType::input_signal_bind &&
         tokens[begin + 1].type_ != TokenType::output_signal_bind &&
         tokens[begin + 1].type_ != TokenType::tag_bind &&
         tokens[begin].type_ != TokenType::trace))
    {
      if (tokens[begin].type_ == TokenType::input_signal_bind ||
          tokens[begin].type_ == TokenType::output_signal_bind)
        errInvalidToken(tokens[begin], "please see signal binding syntax");
      else
        errInvalidToken(tokens[begin], "unable to parse override syntax");
      throw ParserError{};
    }

    attemptOverride(current, begin);
  }

  current.range_.end_ = begin + 1;
  return current;
}

void
    Parser::attemptOverride(Block &current, int &begin)
{
  auto const tokens = lexer_.getTokens();
  switch (tokens[begin + 1].type_)
  {
    case TokenType::word:
      attemptTrace(current, begin);
      break;
    case TokenType::assignment:
      attemptParameterOverride(current, begin);
      break;
    case TokenType::tag_bind:
      attemptTagBindOverride(current, begin);
      break;
    case TokenType::input_signal_bind:
      attemptSignalBindOverride(current, begin, true);
      break;
    case TokenType::output_signal_bind:
      attemptSignalBindOverride(current, begin, false);
      break;
    default:
      errInvalidToken(tokens[begin],
                      "unexpected symbol: expected parameter or tag-rewrite? "
                      "or vector<component> here");
      throw ParserError{};
  }
}

void
    Parser::attemptTagBindOverride(Block &current, int &begin)
{
  auto const tokens = lexer_.getTokens();

  if (tokens[begin].type_ != TokenType::word &&
      tokens[begin].type_ != TokenType::tag_name)
  {
    errInvalidToken(tokens[begin], "expected tag-name here");
    throw ParserError{};
  }

  Block::TokenBlockSignalBind conversion_sequence;
  conversion_sequence.source_ = tokens[begin];

  while (begin + 2 < static_cast<int>(tokens.size()))
  {
    if (tokens[begin + 1].type_ != TokenType::tag_bind)
    {
      errInvalidToken(tokens[begin + 1], "expected => here");
      throw ParserError{};
    }

    if (tokens[begin + 2].type_ == TokenType::component ||
        tokens[begin + 2].type_ == TokenType::variable)
    {
      auto converter_block = expandBlock(begin + 2);
      conversion_sequence.sequence_.push_back(
          { tokens[begin + 2], { converter_block } });
      begin = converter_block.range_.end_ - 1;
    }
    else if (tokens[begin + 2].type_ == TokenType::word ||
             tokens[begin].type_ == TokenType::tag_name)
    {
      conversion_sequence.sink_ = tokens[begin + 2];
      begin += 2;
      break;
    }
    else
    {
      errInvalidToken(
          tokens[begin + 2],
          "expected signal-name or Converter-component here");
      throw ParserError{};
    }
  }

  begin++;

  current.tag_binds_.push_back(conversion_sequence);
}


void
    Parser::attemptSignalBindOverride(Block &current, int &begin, bool is_input)
{
  auto const tokens = lexer_.getTokens();

  auto signal_bind_type =
      is_input ? TokenType::input_signal_bind : TokenType::output_signal_bind;

  if (tokens[begin].type_ != TokenType::word)
  {
    errInvalidToken(
        tokens[begin],
        "expected " +
            (is_input ? std::string{ "input" } : std::string{ "output" }) +
            " signal here");
    throw ParserError{};
  }

  Block::TokenBlockSignalBind conversion_sequence;
  conversion_sequence.source_ = tokens[begin];

  while (begin + 2 < static_cast<int>(tokens.size()))
  {
    if (tokens[begin + 1].type_ != signal_bind_type)
    {
      errInvalidToken(
          tokens[begin + 1],
          "expected " +
              (is_input ? std::string{ "->" } : std::string{ "<-" }) +
              " here");
      throw ParserError{};
    }

    if (tokens[begin + 2].type_ == TokenType::component ||
        tokens[begin + 2].type_ == TokenType::variable)
    {
      auto converter_block = expandBlock(begin + 2);
      conversion_sequence.sequence_.push_back(
          { tokens[begin + 2], { converter_block } });
      begin = converter_block.range_.end_ - 1;
    }
    else if (tokens[begin + 2].type_ == TokenType::word)
    {
      conversion_sequence.sink_ = tokens[begin + 2];
      begin += 2;
      break;
    }
    else
    {
      errInvalidToken(
          tokens[begin + 2],
          "expected signal-name or Converter-component here");
      throw ParserError{};
    }
  }

  begin++;

  if (is_input)
    current.input_signal_binds_.push_back(conversion_sequence);
  else
  {
    std::swap(conversion_sequence.source_, conversion_sequence.sink_);
    rs::reverse(conversion_sequence.sequence_);
    current.output_signal_binds_.push_back(conversion_sequence);
  }
}

void
    Parser::attemptParameterOverride(Block &current, int &begin)
{
  auto const tokens = lexer_.getTokens();
  switch (tokens[begin + 2].type_)
  {
    case TokenType::word:
    case TokenType::tracked_word:
      if (auto f = rs::find_if(current.overrides_,
                               [&](auto over) {
                                 return over.lhs_.expr_ == tokens[begin].expr_;
                               });
          f != rs::end(current.overrides_))
      {
        errInvalidToken(f->lhs_, "parameters already overridden here");
        errInvalidToken(tokens[begin], "cannot override this parameter again");
        throw ParserError{};
      }

      current.overrides_.push_back({ tokens[begin], tokens[begin + 2] });
      begin += 3;
      break;
    case TokenType::open_nested_block_vector:
    {
      auto v = attemptVectorBlock(begin + 2);
      current.nested_vector_.push_back({ tokens[begin], v });
      begin += v.size() + 4;
      break;
    }
    case TokenType::variable:
    case TokenType::component:
      if (auto f = rs::find_if(current.nested_,
                               [&](auto over) {
                                 return over.name_.expr_ == tokens[begin].expr_;
                               });
          f != rs::end(current.nested_))
      {
        errInvalidToken(f->name_, "nested component already overridden here");
        errInvalidToken(tokens[begin],
                        "cannot override this nested component again");
        throw ParserError{};
      }
      current.nested_.push_back({ tokens[begin], {expandBlock(begin + 2)} });
      begin = current.nested_.back().blocks_[0].range_.end_;
      break;
    default:
      errInvalidToken(tokens[begin + 2],
                      "expected override of parameter or nested spec here");
      throw ParserError{};
  }
}

void
    Parser::attemptTrace(Block &current, int &begin)
{
  auto const tokens = lexer_.getTokens();
  if (tokens[begin].type_ != TokenType::trace ||
      tokens[begin + 1].type_ != TokenType::word)
  {
    errInvalidToken(tokens[begin + 1], "expected assignment here");
    throw ParserError{};
  }
  if (tokens[begin + 2].type_ != TokenType::word)
  {
    if (tokens[begin + 2].type_ == TokenType::tracked_word)
      errInvalidToken(
          tokens[begin + 2],
          "traces cannot depend on other tags: must be a literal Number");
    else
      errInvalidToken(tokens[begin + 2],
                      "expected parameter trace number here");
    throw ParserError{};
  }
  current.traces_.push_back({ tokens[begin + 1], tokens[begin + 2] });
  begin += 3;
}

std::vector<Block>
    Parser::attemptVectorBlock(int begin)
{
  auto const tokens = lexer_.getTokens();
  auto       f =
      rs::find_if(rs::begin(tokens) + begin, rs::end(tokens), [](auto token) {
        return token.type_ == TokenType::close_nested_block_vector;
      });
  if (f == rs::end(tokens))
  {
    errInvalidToken(tokens[begin], "dangling vector component");
    throw ParserError{};
  }
  std::vector<Block> vec;
  for (begin++; rs::begin(tokens) + begin != f; begin++)
    if (tokens[begin].type_ != TokenType::variable &&
        tokens[begin].type_ != TokenType::component)
    {
      errInvalidToken(tokens[begin], "nested vector must contain components");
      throw ParserError{};
    }
    else
      vec.push_back(tokens[begin].type_ == TokenType::variable
                        ? variableBlock(begin)
                        : componentBlock(begin));

  return vec;
}

Block
    Parser::variableBlock(int begin)
{
  auto const tokens = lexer_.getTokens();
  if (auto find_block = rs::find_if(variables_,
                                    [tok = tokens[begin]](auto var) {
                                      return var.user_name_.expr_ ==
                                             tok.expr_.substr(1);
                                    });
      find_block == variables_.end())
  {
    errInvalidToken(tokens[begin],
                    "this variable has not been defined",
                    variables_ | rv::transform([](auto var) {
                      return var.user_name_.expr_;
                    }) | rs::to<std::vector<std::string>>);
    throw ParserError{};
  }
  else
  {
    return find_block->user_specified_block_;
  }
}

Block
    Parser::componentBlock(int begin)
{
  auto const tokens = lexer_.getTokens();
  Block      current;
  current.name_       = tokens[begin].expr_;
  current.name_token_ = tokens[begin];

  return current;
}

void
    Parser::parseFromLexer()
{
  auto const tokens = lexer_.getTokens();
  for (auto start = 0u; start != tokens.size();
       start      = variables_.back().user_specified_block_.range_.end_)
    parseExpression(start);

  return;
}

std::optional<Parser::VariedParameterPosition>
    Parser::hasVariedParameter()
{
  auto const tokens = lexer_.getTokens();

  auto open_variance_position =
      rs::find(tokens, TokenType::open_varied_argument, &Token::type_);
  auto close_variance_position =
      rs::find(tokens, TokenType::close_varied_argument, &Token::type_);
  if (open_variance_position == rs::end(tokens) &&
      open_variance_position == rs::end(tokens))
    return std::nullopt;

  if (open_variance_position < rs::begin(tokens) + 2 ||
      (open_variance_position - 1)->type_ != TokenType::assignment ||
      (open_variance_position - 2)->type_ != TokenType::word)
  {
    errInvalidToken(*open_variance_position,
                    "varied parameter syntax error here");
    throw ParserError{};
  }

  if (close_variance_position == rs::end(tokens))
  {
    errInvalidToken(*open_variance_position, "varied parameter is not closed");
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

  return VariedParameterPosition{
    static_cast<int>(open_variance_position - rs::begin(tokens)),
    static_cast<int>(close_variance_position - rs::begin(tokens))
  };
}

std::vector<Parser>
    Parser::varyParameter()
{
  auto pos = hasVariedParameter();
  if (!pos)
  {
    return { *this };
  }

  auto               tokens = lexer_.getTokens();
  std::vector<Token> subs;
  rs::copy(rs::begin(tokens) + pos->open_ + 1,
           rs::begin(tokens) + pos->close_,
           rs::back_inserter(subs));

  tokens.erase(rs::begin(tokens) + pos->open_,
               rs::begin(tokens) + pos->close_ + 1);

  return subs | rv::transform([&](auto token) {
           auto lexer       = lexer_;
           auto temp_tokens = tokens;
           temp_tokens.insert(rs::begin(temp_tokens) + pos->open_, token);
           lexer.updateTokens(temp_tokens);
           Parser p{ *this };
           p.updateLexer(lexer);
           p.labels_.push_back(
               { (rs::begin(temp_tokens) + pos->open_ - 2)->expr_,
                 token.expr_ });
           return p;
         }) |
         rs::to<std::vector<Parser>>;
}

// debug only
void
    Parser::print(Block b)
{
  std::cout << b.name_ << " [" << b.range_.begin_ << "," << b.range_.end_
            << "]\n";
  for (auto [name, value] : b.overrides_)
    std::cout << name.expr_ << ":" << value.expr_ << "\n";
  for (auto [name, value] : b.nested_)
  {
    std::cout << name.expr_ << "->\n";
    print(value[0]);
  }
}
void
    Parser::resolveTrackedWords()
{

  for (auto &var : variables_)
  {
    replaceTokenWord(var.user_specified_block_);
  }
}

void
    Parser::replaceTokenWord(Block &block)
{
  for (auto &over : block.overrides_)
  {
    if (!over.rhs_.refers_.empty())
      over.rhs_.expr_ = lookUpTokenWord(over.rhs_);
  }
  {
    for (auto &nested : block.nested_)
      replaceTokenWord(nested.blocks_[0]);
  }
}

std::string
    Parser::lookUpTokenWord(Token token)
{
  auto refers = token.refers_.substr(1);
  auto pats   = refers | rv::split('-') | rs::to<std::vector<std::string>>;

  auto find_block = rs::find_if(
      variables_, [&](auto var) { return var.user_name_.expr_ == pats[0]; });
  if (find_block == rs::end(variables_))
  {
    errInvalidToken(token, "tracked path " + refers + " is not valid");
    throw ParserError{};
  }

  auto block = find_block->user_specified_block_;
  for (auto i = 1u; i < pats.size() - 1; i++)
  {
    auto nested = pats[i];

    auto nb = rs::find_if(block.nested_,
                          [&](auto var) { return var.name_.expr_ == nested; });
    if (nb == rs::end(block.nested_))
    {
      errInvalidToken(token,
                      "tracked path " + refers + " is not valid: '" + nested +
                          "' is not a nested component");
      throw ParserError{};
    }
    block = nb->blocks_[0];
  }

  auto parameter = pats.back();

  auto par = rs::find_if(block.overrides_,
                         [&](auto var) { return var.lhs_.expr_ == parameter; });
  if (par == rs::end(block.overrides_))
  {
    errInvalidToken(token,
                    "tracked path " + refers + " is not valid: '" + parameter +
                        "' is not a parameter");
    throw ParserError{};
  }
  return par->rhs_.expr_;
}
}   // namespace language
}   // namespace ded
