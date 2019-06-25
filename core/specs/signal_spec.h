
#pragma once

#include <range/v3/all.hpp>
#include <string>
#include <vector>

namespace life {

class signal_spec {
private:
  std::string user_name_;
  std::string additional_identifier_;
  std::string full_type_;

  std::string vector_type_;
  std::string user_parameter_;
  long        size_ = 0;
  bool is_any_type_ = false, is_vector_ = false, is_any_vector_size_ = false,
       is_user_set_vector_size_ = false;

public:
  signal_spec() = default;

  signal_spec(std::string sig);

  signal_spec(std::string name, std::string idt);

  signal_spec(std::string name, std::string id, std::string type);

  auto full_name() const
  {
    return user_name_ + ":" + additional_identifier_ + "-" + full_type_;
  }
  /*
  auto id_type_specifier() const
  {
    return additional_identifier_ + "-" + full_type_;
  }
  */
  auto user_name() const { return user_name_; }
  auto identifier() const { return additional_identifier_; }
  void update_identifier(std::string id) { additional_identifier_ = id; }
  auto type() const { return full_type_; }
  auto user_parameter() const { return user_parameter_; }

  void instantiate_user_parameter( long size) { size_ = size; }

  bool exactly_matches(signal_spec);
  bool convertible_to(signal_spec);
};

struct io_signals
{
  std::vector<std::pair<std::string, signal_spec>> inputs_;
  std::vector<std::pair<std::string, signal_spec>> outputs_;
};

struct tags
{
  std::vector<std::pair<std::string, signal_spec>> pre_;
  std::vector<std::pair<std::string, signal_spec>> post_;
};
}   // namespace life
