
#pragma once

#include <regex>
#include <string>
#include <vector>

#include "token.hpp"

namespace ded
{
namespace language
{
// order of options matter
inline static const std::regex valid_symbol_{
  R"~~(^(\s+|\[|\]|\=>|->|<-|<|>|\{|\}|\=|\?|\$\w+|\!\w+|\^[-\w]+|\w+\:\w+|"[^"]*"|[\w\.]+))~~"
};

class Lexer
{
private:
  void                     openFile(std::string);
  void                     lexTokens();
  std::string              file_name_;
  std::vector<std::string> lines_;
  std::vector<Token>       tokens_;

public:
  Lexer() = default;
  Lexer(std::string);
  void
      updateTokens(std::vector<Token> tokens)
  {
    tokens_ = tokens;
  }
  std::vector<Token>
      getTokens() const
  {
    return tokens_;
  }
  std::vector<std::string>
      getLines() const
  {
    return lines_;
  }
  std::string
      getFileName() const
  {
    return file_name_;
  }
};

}   // namespace language
}   // namespace ded
