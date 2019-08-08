
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
std::vector<std::string> all_component_names();
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


  void err_invalid_token(Token, std::string, std::vector<std::string> = {});
  /*
  void open_file(std::string);

  void lex();
  void err_unknown_symbol(std::pair<int, int>);
	*/

  void parse_expression(int);

  Block expand_block(int);

  Block component_block(int);
  Block variable_block(int);

  Block process_overrides(Block, int);

  void attempt_override(Block &, int &);
  void attempt_parameter_override(Block &, int &);
  void attempt_trace(Block &, int &);

  void replace_tw(Block& );
  std::string look_up_tw(Token);

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

  std::vector<Parser> vary_parameter();
  std::optional<std::pair<int, int>> has_varied_parameter();
  void print(Block b);

  void
      update_source_tokens(SourceTokens s)
  {
    source_tokens_ = s;
  }

  void
      update_labels(std::pair<std::string, std::string> label)
  {
    labels_.push_back(label);
  }

  void resolve_tracked_words();

  friend class specs::EnvironmentSpec;
  friend class specs::EntitySpec;
  friend class specs::PopulationSpec;
};
}   // namespace language
}   // namespace ded
