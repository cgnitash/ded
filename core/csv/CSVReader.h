
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>
#include <set>
#include <sstream>
#include <vector>

// can parse a csv string into a vector of strings
// The delimiter and quotation character can be specified:
// by default  , and "
// works by using Transition to manage state transitions
// (the state-to-state transition table) and appending
// characters to strings as needed.

class CSVReader {

  char delimiter_ = ',', quotation_ = '"';
  std::string current_string_;
  std::vector<std::string> fields_;
  enum class input { chars, delim, quote, wh_sp };
  enum class state {
    precw, // leading whitespace
    field, // non quoted field
    delim, // found delim
    quote, // reading quoted field
    succw, // trailing whitespace
    openq, // first quote of quoted field
    CRASH
  };

  // cast state to int to index into more
  // readable state-to-state transition table
  std::array<std::array<state, 4>, 6> Transition = {{
      //  chars          delim         quote         wh_sp
      {state::field, state::delim, state::quote, state::precw}, // precw 0
      {state::field, state::delim, state::succw, state::succw}, // field 1
      {state::field, state::delim, state::quote, state::precw}, // delim 2
      {state::openq, state::openq, state::succw, state::openq}, // quote 3
      {state::CRASH, state::delim, state::CRASH, state::succw}, // succw 4
      {state::openq, state::openq, state::succw, state::openq}  // openq 5
      //{state::CRASH, state::CRASH, state::CRASH, state::CRASH}  // CRASH 6
  }};

  CSVReader::input symbol(char c);

  void doStateAction(state s, char c, const std::string &line,
                     const int &charIndex);

  void showLineAndErrorChar(const std::string &line, const int &charIndex);

public:
  CSVReader() = default;
  CSVReader(char d) : delimiter_(d) {}
  CSVReader(char d, char oq) : delimiter_(d), quotation_(oq) {}
  std::vector<std::string> parseLine(const std::string &s);
};

