

#include "parser.h"
#include "../term_colours.h"
#include <fstream>
#include <regex>
#include <string>
#include <vector>

namespace life {

const std::regex parser::valid_symbol_{ R"~~(^(\s+|\{|\}|\w+|\$\w+|\=))~~" };

void
    parser::open_file(std::string file_name)
{

  std::ifstream file(file_name);
  if (!file.is_open())
  {
    std::cout << "error: " << file_name << " not found\n";
    throw parser_error{};
  }

  std::string line;
  while (std::getline(file, line)) lines_.push_back(line);
}

void
    parser::lex()
{
  for (auto [line_number, line] : ranges::view::enumerate(lines_))
  {
    line.erase(ranges::find(line, '#'), line.end());

    auto i = line.cbegin();
    for (std::smatch m; i != line.cend() &&
                        std::regex_search(i, line.cend(), m, valid_symbol_);
         i += m.str().length())
      if (!ranges::all_of(m.str(), ::isspace))
        tokens_.push_back(token{ parse_token_type(m.str()),
                                 m.str(),
                                 { line_number + 1, i - line.cbegin() + 1 } });

    if (i != line.cend())
      err_unknown_symbol({ line_number, i - line.cbegin() });
  }

  for (auto e : tokens_)
    std::cout << "symbol :" << e.expr_ << " at position " << e.location_.first
              << "," << e.location_.second << std::endl;
}

void
    parser::err_unknown_symbol(std::pair<int, int> location)
{

  std::cout << "parse-error:" << location.first + 1 << ":"
            << location.second + 1 << "\n"
            << lines_[location.first] << "\n"
            << std::string(location.second, ' ') << term_colours::red_fg
            << "^ unknown symbol\n"
            << term_colours::reset;
  throw parser_error{};
}

void
    parser::parse_expression(int begin)
{
  if (begin + 2 >= static_cast<int>(tokens_.size()))
  {
    err_invalid_token(tokens_[begin], "unable to parse expression syntax");
    throw parser_error{};
  }

  if (tokens_[begin].type_ != token_type::word)
  {
    err_invalid_token(tokens_[begin], "expected new variable name here");
    throw parser_error{};
  }

  if (auto prev = ranges::find_if(variables_,
                                  [tok = tokens_[begin]](auto var) {
                                    return var.first.expr_ == tok.expr_;
                                  });
      prev != variables_.end())
  {
    err_invalid_token(tokens_[begin], "variable re-definition not allowed");
    err_invalid_token(prev->first, "variable already defined here");
    throw parser_error{};
  }

  if (tokens_[begin + 1].type_ != token_type::assignment)
    err_invalid_token(tokens_[begin + 1], "expected =");

  if (tokens_[begin + 2].type_ != token_type::component &&
      tokens_[begin + 2].type_ != token_type::variable)
  {
    err_invalid_token(tokens_[begin + 2],
                      "expected existing variable name or component here");
    throw parser_error{};
  }

  variables_.push_back({ tokens_[begin], expand_block(begin + 2) });
  variables_.back().second.range_.first = begin + 2;
}

void
    parser::err_invalid_token(token tok, std::string message)
{
  std::cout << "parse-error\n"
            << std::setw(3) << tok.location_.first + 1 << ":"
            << lines_[tok.location_.first] << "\n"
            << std::string(tok.location_.second + 4, ' ')
            << term_colours::red_fg << std::string(tok.expr_.length(), '~')
            << "\n"
            << std::string(tok.location_.second + 4, ' ') << "^ " << message
            << "\n";
}

block
    parser::expand_block(int begin)
{

  auto current =
      tokens_[begin].type_ == token_type::variable
          ? ranges::find_if(variables_,
                            [tok = tokens_[begin]](auto var) {
                              return var.first.expr_ == tok.expr_.substr(1);
                            })
                ->second
          : component_block(begin);

    current.range_.first = begin;

  if (begin + 1 == static_cast<int>(tokens_.size()) ||
      tokens_[begin + 1].type_ != token_type::open_brace)
  {
    current.range_.second = begin + 1;
    return current;
  }

  return process_overrides(current, begin + 2);
}

block
    parser::process_overrides(block current, int begin)
{

  for (;;)
  {
    if (begin == static_cast<int>(tokens_.size()))
    {
      err_invalid_token(tokens_[begin - 1], "unmatched brace");
      throw parser_error{};
    }
    if (tokens_[begin].type_ == token_type::close_brace)
    {
      break;
    }

    if (begin + 3 >= static_cast<int>(tokens_.size()) ||
        tokens_[begin + 1].type_ != token_type::assignment)
    {
      err_invalid_token(tokens_[begin], "unable to parse override syntax");
      throw parser_error{};
    }

    switch (tokens_[begin + 2].type_)
    {
      case token_type::word:
        current.overrides_.push_back({ tokens_[begin], tokens_[begin + 2] });
         begin += 3;
			 break;
      case token_type::variable:
      case token_type::component:
        current.nested_.push_back(
            { tokens_[begin],
              expand_block(begin + 2) });
         begin = current.nested_.back().second.range_.second;
		break;
      default:
        err_invalid_token(tokens_[begin + 2], "expected override of spec here");
        throw parser_error{};
    }
  }
  current.range_.second = begin + 1;
  return current;
}

block
    parser::component_block(int begin)
{
  block result;
  auto  var = tokens_[begin].expr_.substr(1);
  for (auto [name, spec] : life::all_entity_specs)
    if (name == var)
    {
      result.name_ = var;
      result.spec_ = spec;
      result.type_ = spec_type::entity;
    }
  for (auto [name, spec] : life::all_environment_specs)
    if (name == var)
    {
      result.name_ = var;
      result.spec_ = spec;
      result.type_ = spec_type::environment;
    }
  for (auto [name, spec] : life::all_population_specs)
    if (name == var)
    {
      result.name_ = var;
      result.spec_ = spec;
      result.type_ = spec_type::population;
    }

  return result;
}

std::pair<population_spec, environment_spec>
    parser::parse(std::string file_name)
{

  std::pair<population_spec, environment_spec> p;
  open_file(file_name);
  lex();
  // match_braces();
  // check_syntax();
  // flatten_blocks();
  if (!tokens_.empty()) parse_expression(0);

  for (auto [name, bl] : variables_) {
	  std::cout << name.expr_ << "\n";
	  print(bl);
  }

  return p;
}
void parser::print(block b) {
	std::cout << b.name_ << " [" << b.range_.first << "," << b.range_.second << "]\n";
	for (auto [name, value] : b.overrides_)
		std::cout << name.expr_ << ":" << value.expr_ << "\n";
	for (auto [name, value] : b.nested_) {
		std::cout << name.expr_ << "->\n";
		print(value);
	}

	
  }
}   // namespace life

