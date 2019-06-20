
#pragma once

#include <string>
#include <vector>

namespace life {

class signal_spec {
private:
  std::string user_name_;
  std::string identifier_;
  std::string full_type_;

  std::string vector_type_;
  std::string user_parameter_;
  long        size_ = 0;
  bool is_vector_ = false, is_any_type_ = false, is_any_vector_= false;

public:
  signal_spec() = default;

  signal_spec(std::string sig) ;

  signal_spec(std::string name, std::string idt);

  signal_spec(std::string name, std::string id, std::string type);

  auto full_name() const
  {
    return user_name_ + ":" + identifier_ + "-" + full_type_;
  }
  auto id_type_specifier() const { return identifier_ + "-" + full_type_; }
  auto user_name() const { return user_name_ ; }
  auto identifier() const { return identifier_ ; }
  auto type() const { return  full_type_; }
};
}   // namespace life
