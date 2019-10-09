
#include <fstream>
#include <iomanip>
#include <iostream>

#include "lexer.hpp"
#include "../utilities/term_colours.hpp"
#include "../utilities/utilities.hpp"

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
    {
      auto expr = m.str();
      if (!rs::all_of(expr, ::isspace))
      {
        auto column             = i - line.cbegin();
        auto diagnostic_message = line;
        diagnostic_message.insert(column + expr.length(),
                                  utilities::TermColours::reset);
        diagnostic_message.insert(column, utilities::TermColours::red_fg);
        std::stringstream diagnostic;
        diagnostic << utilities::TermColours::cyan_fg << "Line" << std::setw(4)
                   << line_number + 1 << ": " << utilities::TermColours::reset
                   << diagnostic_message;
        auto type = language::parseTokenType(expr);
        tokens_.push_back(language::Token{
            type,
            expr,
            { line_number, column },
            diagnostic.str(),
            type == language::TokenType::tracked_word ? expr : std::string{} });
      }
    }

    if (i != line.cend())
    {

      std::cout << "parse-error:\n\n"
                << utilities::TermColours::cyan_fg << "Line" << std::setw(4)
                << line_number + 1 << ": " << utilities::TermColours::reset
                << lines_[line_number] << "\n"
                << std::string(i - line.cbegin() + 10, ' ')
                << utilities::TermColours::red_fg << "^ unknown symbol\n"
                << utilities::TermColours::reset;
      throw language::ParserError{};
    }
  }
}

}   // namespace language
}   // namespace ded
