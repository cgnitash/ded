
#pragma once

#include <regex>
#include <string>
#include <vector>

#include "lexer.hpp"

namespace ded
{

namespace language
{

struct Label
{
  std::string name_{}, value_{};
};
using Labels = std::vector<Label>;

class Parser
{

  Lexer lexer_;

  struct UserVariable
  {
	  Token user_name_{};
	  Block user_specified_block_{};
  };
  std::vector<UserVariable> variables_;

  Labels labels_;

  void parseExpression(int);

  Block expandBlock(int);

  Block componentBlock(int);
  Block variableBlock(int);

  Block processOverrides(Block, int);

  void attemptOverride(Block &, int &);
  void attemptParameterOverride(Block &, int &);
  void attemptSignalBindOverride(Block &, int &);
  void attemptTrace(Block &, int &);
  std::vector<Block> attemptVectorBlock(int);

  void replaceTokenWord(Block& );
  std::string lookUpTokenWord(Token);

public:

  auto
      labels() const
  {
    return labels_;
  }
  auto
      lines() const
  {
    return lexer_.getLines();
  }
  auto
      file_name() const
  {
    return lexer_.getFileName();
  }

  auto
      source_tokens() const
  {
    return lexer_;
  }
  std::vector<UserVariable>
      variables() const
  {
    return variables_;
  }

  void parseFromLexer();

  std::vector<Parser> varyParameter();
  std::optional<std::pair<int, int>> hasVariedParameter();
  void print(Block b);

  void
      updateLexer(Lexer s)
  {
    lexer_ = s;
  }

  void
      updateLabels(std::pair<std::string, std::string> label)
  {
    labels_.push_back({label.first, label.second});
  }

  void resolveTrackedWords();

};
}   // namespace language
}   // namespace ded
