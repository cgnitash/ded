

#pragma once

#include <memory>
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
  close_nested_block_vector,
  input_signal_bind_,
  output_signal_bind_
  //signal_bind
};

TokenType parseTokenType(std::string);

struct Token
{
  TokenType   type_{};
  std::string expr_{};
  struct TokenLocation
  {
    int line_{}, column_{};
  } location_{};
  std::string diagnostic_{};
  std::string refers_{};
};

void errInvalidToken(Token                    token,
                     std::string              message,
                     std::vector<std::string> suggestions = {});

struct TokenAssignment
{
  Token lhs_{}, rhs_{};
};

struct Block
{
  Token       name_token_{};
  std::string name_{};
  struct BlockRange
  {
    int begin_{}, end_{};
  } range_{};
  std::vector<TokenAssignment> overrides_{};
  //std::vector<TokenAssignment> signal_binds_{};
  std::vector<TokenAssignment> traces_{};
  struct TokenBlocks
  {
    Token                    name_{};
    std::vector<Block> blocks_{};
  };
  std::vector<TokenBlocks>  nested_{};
  std::vector<TokenBlocks> nested_vector_{};
  struct TokenBlockSignalBind 
  {
	  Token source_;
	  Token sink_;
	  std::vector<TokenBlocks> sequence_;
  };
  std::vector<TokenBlockSignalBind> input_signal_binds_{};
  std::vector<TokenBlockSignalBind> output_signal_binds_{};
};

}   // namespace language
}   // namespace ded
