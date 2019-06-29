

#include "csv_reader.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <range/v3/all.hpp>
#include <regex>
#include <set>
#include <sstream>
#include <vector>

namespace ded {
namespace utilities {
namespace csv {
CSVReader::input
    CSVReader::symbol(char c)
{
  return c == delimiter_ ? input::delim
                         : c == quotation_ ? input::quote
                                           : c == ' ' ? input::wh_sp
                                                      : /* char */ input::chars;
}

void
    CSVReader::error_message(const std::string &line, const int &charIndex)
{
  // example output:
  // this, was, your string, with, error
  //                ^
  std::cout << line << std::endl;
  std::cout << std::string(std::max(charIndex - 1, 0), ' ') << "^" << std::endl;
}

void
    CSVReader::action(state              s,
                      char               c,
                      const std::string &line,
                      const int &        char_index)
{
  switch (s)
  {
    case state::precw:
      // leading whitespace
      // do nothing
      break;
    case state::field:
      // non nested field
      // simply keep reading
      current_string_ += c;
      break;
    case state::delim:
      // found delim
      // add field
      fields_.push_back(current_string_);
      // clear field
      current_string_.clear();
      break;
    case state::quote:
      // ready to start
      // reading quoted field
      break;
    case state::openq:
      // nested field
      // simply keep reading
      current_string_ += c;
      break;
    case state::succw:
      // trailing whitespace
      // do nothing
      break;
    case state::CRASH:
      std::cout << "cannot parse : unexpected character" << std::endl;
      error_message(line, char_index);
      exit(1);
      break;
  }
}

std::vector<std::string>
    CSVReader::parse_line(const std::string &s)
{
  fields_.clear();
  state curr = state::precw;
  //auto  index{ 0 };
  for (auto [i, c] : ranges::view::enumerate(s))
  {
    // cast to int to index into more
    // readable transition table
    curr = Transition[static_cast<int>(curr)][static_cast<int>(symbol(c))];

    action(curr, c, s, i);
  }

  if (curr == state::openq)
  {
    std::cout << "cannot parse : missing quotation" << std::endl
              << s << std::endl;
    std::exit(1);
  }
  // add the final field
  fields_.push_back(current_string_);
  current_string_.clear();
  return fields_;
}
}}}
