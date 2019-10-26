

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>
#include <set>
#include <sstream>
#include <vector>

#include "csv.hpp"

namespace ded
{
namespace utilities
{
namespace csv
{
std::vector<std::string>
    CSV::single_column(std::string column)
{
  auto const column_index = rs::find(column_names_, column);

  if (column_index == rs::end(column_names_))
  {
    std::cout << " Error : could not find column " << column
              << " to merge from file " << file_name_ << std::endl;
    std::exit(1);
  }

  return rows_ |
         rv::transform([c = column_index - rs::begin(column_names_)](auto row) {
           return row[c];
         }) |
         rs::to<std::vector<std::string>>;
}

std::string
    CSV::look_up(std::string lookup_column,
                 std::string value,
                 std::string return_column) const
{

  // find position of lookup column
  auto const column_iter = rs::find(column_names_, lookup_column);
  if (column_iter == rs::end(column_names_))
  {
    std::cout << " Error : could not find column " << lookup_column
              << " in file " << file_name_ << std::endl;
    std::exit(1);
  }

  // find position of return column
  auto const return_iter = rs::find(column_names_, return_column);
  if (return_iter == rs::end(column_names_))
  {
    std::cout << " Error : could not find column " << return_column
              << " in file " << file_name_ << std::endl;
    std::exit(1);
  }

  // find number of values in lookup column
  auto const value_count = rs::count_if(
      rows_, [value, c = column_iter - rs::begin(column_names_)](auto row) {
        return row[c] == value;
      });

  if (!value_count)
  {
    std::cout << "Error : could not find requested lookup value" << value
              << " from column " << lookup_column << " from file " << file_name_
              << std::endl;
    std::exit(1);
  }

  if (value_count > 1)
  {
    std::cout << "Error : multiple entries found for requested lookup value"
              << value << " from column " << lookup_column << " from file "
              << file_name_ << std::endl;
    std::exit(1);
  }

  // find row where lookup column has value
  auto const value_index =
      rs::find_if(rows_,
                  [value, c = column_iter - rs::begin(column_names_)](
                      auto row) { return row[c] == value; }) -
      rs::begin(rows_);

  return rows_[value_index][return_iter - rs::begin(column_names_)];
}

CSV::CSV(std::string fn, char s, char se) : file_name_(fn), reader_(s, se)
{

  std::ifstream file(file_name_);
  if (!file.is_open())
  {
    std::cout << " Error: cannot open csv file " << file_name_ << std::endl;
    std::exit(1);
  }

  std::string raw_line;

  // read header line
  getline(file, raw_line);
  column_names_ = reader_.parse_line(raw_line);

  // ensure column names are unique
  auto uniq_headers = column_names_ | rs::copy | ra::sort | ra::unique;

  if (uniq_headers.size() != column_names_.size())
  {
    std::cout << "Error: CSV file " << file_name_
              << " does not have unique Header names" << std::endl;
    std::exit(1);
  }

  // read remaining rows
  while (getline(file, raw_line))
  {
    auto data_line = reader_.parse_line(raw_line);
    // ensure all rows have correct number of columns
    if (column_names_.size() != data_line.size())
    {
      std::cout << " Error: incorrect number of columns in CSV file "
                << file_name_ << std::endl;
      std::exit(1);
    }
    rows_.push_back(data_line);
  }
}

void
    CSV::merge(CSV merge_csv, std::string column)
{

  if (!has_column(column))
  {
    std::cout << " Error : could not find column " << column << " in file "
              << file_name_ << std::endl;
    std::exit(1);
  }

  if (!merge_csv.has_column(column))
  {
    std::cout << " Error : could not find column " << column
              << " to merge from file " << merge_csv.file_name() << std::endl;
    std::exit(1);
  }

  // ensure lookup column has distinct values
  auto lookup_values = single_column(column) | ra::sort | ra::unique;
  if (lookup_values.size() != rows_.size())
  {
    std::cout << "Error: CSV file " << file_name_
              << " does not have unique values in column " << column
              << std::endl;
    std::exit(1);
  }

  // ensure column in second file has matching values for all values in this
  // file and are unique
  auto merge_values = merge_csv.single_column(column) | ra::sort | ra::unique;
  if (merge_values.size() != merge_csv.single_column(column).size())
  {
    std::cout << "Error: CSV file " << merge_csv.file_name()
              << " needs to have unique values in column " << column
              << std::endl;
    std::exit(1);
  }
  if (!rs::includes(lookup_values, merge_values))
  {
    std::cout << "Error: CSV file " << merge_csv.file_name()
              << " does not have some matching values for column " << column
              << std::endl;
    std::exit(1);
  }

  // find position of lookup column
  auto const column_index =
      rs::find(column_names_, column) - rs::begin(column_names_);

  // merge columns from second file
  for (auto const &merge_column : merge_csv.column_names())
  {
    // only add additional columns
    if (rs::find(column_names_, merge_column) == rs::end(column_names_))
    {
      column_names_.push_back(merge_column);
      // for each column add value to every row
      for (auto &row : rows_)
        row.push_back(
            merge_csv.look_up(column, row[column_index], merge_column));
    }
  }
}

}   // namespace csv
}   // namespace utilities
}   // namespace ded
