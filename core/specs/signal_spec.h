
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
  long        size_      = 0;
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
  auto id_type_specifier() const { return additional_identifier_ + "-" + full_type_; }
  auto user_name() const { return user_name_; }
  auto identifier() const { return additional_identifier_; }
  auto type() const { return full_type_; }
  void instantiate_user_parameter(std::string, long);
  bool exactly_matches(signal_spec);
  bool convertible_to(signal_spec);
};

struct io_signals
{
  std::vector<std::pair<std::string, signal_spec>> inputs_;
  std::vector<std::pair<std::string, signal_spec>> outputs_;

  /*
  void set_input(std::string name, signal_spec i)
  {
    inputs_.push_back({ name, i });
  }

  signal_spec get_input(std::string name)
  {
    return ranges::find_if(inputs_, [=](auto ns) { return ns.first == name; })
        ->second;
  }

  void set_output(std::string name, signal_spec o)
  {
    outputs_.push_back({ name, o });
  }

  signal_spec get_output(std::string name)
  {
    return ranges::find_if(outputs_, [=](auto ns) { return ns.first == name; })
        ->second;
  }
  */
};
}   // namespace life
