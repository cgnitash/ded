
#pragma once

#include <regex>
#include <string>
#include <vector>

#include "lexer.hpp"

namespace ded
{

// forward declaration to provide friendship
namespace config_manager
{
std::vector<std::string> allComponentNames();
}
// forward declaration to provide friendship
namespace specs
{
class SubstrateSpec;
class ProcessSpec;
class PopulationSpec;
class EncodingSpec;
}   // namespace specs

namespace language
{

using Labels = std::vector<std::pair<std::string, std::string>>;

class Parser
{

  Lexer lexer_;

  std::vector<std::pair<Token, Block>> variables_;

  Labels labels_;


//  void errInvalidToken(Token, std::string, std::vector<std::string> = {});

  void parseExpression(int);

  Block expandBlock(int);

  Block componentBlock(int);
  Block variableBlock(int);

  Block processOverrides(Block, int);

  void attemptOverride(Block &, int &);
  void attemptParameterOverride(Block &, int &);
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
  std::vector<std::pair<Token, Block>>
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
    labels_.push_back(label);
  }

  void resolveTrackedWords();

  friend class specs::ProcessSpec;
  friend class specs::SubstrateSpec;
  friend class specs::PopulationSpec;
  friend class specs::EncodingSpec;
};
}   // namespace language
}   // namespace ded
