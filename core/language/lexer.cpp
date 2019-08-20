
#include "lexer.h"
#include "../utilities/term_colours.h"
#include "../utilities/utilities.h"
#include <fstream>
#include <iostream>

namespace ded
{
namespace language
{

Lexer::Lexer(std::string file_name)
{
  openFile(file_name);
  lexTokens();
}

void
    Lexer::openFile(std::string file_name)
{
  file_name_ = file_name;
  std::ifstream file(file_name);
  if (!file.is_open())
  {
    std::cout << "error: " << file_name << " not found\n";
    throw language::ParserError{};
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
    Lexer::lexTokens()
{
  // order of options matter
  const std::regex valid_symbol_{
    R"~~(^(\s+|\[|\]|<|>|\{|\}|\=|\?|\$\w+|\!\w+|\^[-\w]+|"[^"]*"|[\w\.]+))~~"
  };

  for (auto [line_number, line] : rv::enumerate(lines_))
  {
    line.erase(rs::find(line, '#'), line.end());

    auto i = line.cbegin();
    for (std::smatch m; i != line.cend() &&
                        std::regex_search(i, line.cend(), m, valid_symbol_);
         i += m.str().length())
      if (!rs::all_of(m.str(), ::isspace))
      {
        auto type = language::parseTokenType(m.str());
        tokens_.push_back(language::Token{
            type,
            m.str(),
            { line_number, i - line.cbegin() },
            type == language::TokenType::tracked_word ? m.str()
                                                      : std::string{} });
      }
    if (i != line.cend())
    {

      std::cout << "parse-error:" << line_number + 1 << ":"
                << i - line.cbegin() + 1 << "\n"
                << lines_[line_number] << "\n"
                << std::string(i - line.cbegin(), ' ')
                << utilities::TermColours::red_fg << "^ unknown symbol\n"
                << utilities::TermColours::reset;
      throw language::ParserError{};
    }
  }
}

}   // namespace language
}   // namespace ded
