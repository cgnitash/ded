
#pragma once

#include "token.h"
#include <regex>
#include <string>
#include <vector>

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
class EntitySpec;
class EnvironmentSpec;
class PopulationSpec;
}   // namespace specs

namespace language
{

using Labels = std::vector<std::pair<std::string, std::string>>;

struct SourceTokens
{
  std::string                          file_name;
  std::vector<std::string>             lines;
  std::vector<Token>                   tokens;
};

class Parser
{

  SourceTokens source_tokens_;

  std::vector<std::pair<Token, Block>> variables_;

  Labels labels_;


  void errInvalidToken(Token, std::string, std::vector<std::string> = {});
  /*
  void openFile(std::string);

  void lexTokens();
  void err_unknown_symbol(std::pair<int, int>);
	*/

  void parseExpression(int);

  Block expandBlock(int);

  Block componentBlock(int);
  Block variableBlock(int);

  Block processOverrides(Block, int);

  void attemptOverride(Block &, int &);
  void attemptParameterOverride(Block &, int &);
  void attemptTrace(Block &, int &);

  void replaceTokenWord(Block& );
  std::string lookUpTokenWord(Token);

public:
  const static std::regex valid_symbol_;
  //Parser() = default;

  auto
      labels() const
  {
    return labels_;
  }
  auto
      lines() const
  {
    return source_tokens_.lines;
  }
  auto
      file_name() const
  {
    return source_tokens_.file_name;
  }
  auto
      source_tokens() const
  {
    return source_tokens_;
  }
  std::vector<std::pair<Token, Block>>
      variables() const
  {
    return variables_;
  }

  void parse(SourceTokens);

  std::vector<Parser> varyParameter();
  std::optional<std::pair<int, int>> hasVariedParameter();
  void print(Block b);

  void
      updateSourceTokens(SourceTokens s)
  {
    source_tokens_ = s;
  }

  void
      updateLabels(std::pair<std::string, std::string> label)
  {
    labels_.push_back(label);
  }

  void resolveTrackedWords();

  friend class specs::EnvironmentSpec;
  friend class specs::EntitySpec;
  friend class specs::PopulationSpec;
};
}   // namespace language
}   // namespace ded
