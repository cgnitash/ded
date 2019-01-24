
#pragma once

#include "CSVReader.h"

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

// parses a csv file and stores in memory.
// the first line of the file is treated as the column headers
// The delimiter and quotation character can be specified:
// by default  , and "
class CSV {

  std::string file_name_;

  // to do the reading
  CSVReader reader_;
  // the 1st row; column headers
  std::vector<std::string> column_names_;
  // remaining rows of the file
  std::vector<std::vector<std::string>> rows_;

public:
  CSV(std::string fn, char s, char se);
  CSV(std::string fn) : CSV(fn, ',', '"') {}

  // return csv file name
  std::string fileName() const { return file_name_; }

  // number of columns in the file
  auto column_count() const { return column_names_.size(); }

  // number of rows in the file
  auto row_count() const { return rows_.size(); }

  // return all columns in the file
  std::vector<std::string> column_names() const { return column_names_; }

  // return all rows in the file
  std::vector<std::vector<std::string>> rows() const { return rows_; }

  // return all values corresponding to a single column
  std::vector<std::string> singleColumn(std::string column);

  // look up a value in a column and return the value in the corresponding row
  // of the other column
  std::string lookUp(std::string lookup_column,
                     std::string value,
                     std::string return_column) const;

  // merges another csv file. Only extra columns are added. All values in the
  // specified column must also exist in the same column in the other file.
  // only rows in the other file that have matching values in this
  // file are merged
  void merge(CSV merge_csv, std::string column);

  // check existence of a column
  bool hasColumn(std::string name) const;
};
