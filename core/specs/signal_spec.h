
#pragma once

#include <string>
#include <vector>

namespace life {

class signal_spec {
private:
  std::string user_name_;
  std::string identifier_;

  std::string type_;
  std::string user_parameter_;
  long        size_ = 0;
  bool is_vector_ = false, is_any_type_ = false, is_any_vector_= false;

public:
  signal_spec() = default;

  signal_spec(std::string sig) {
	  auto p = sig.find(':');
	  signal_spec(sig.substr(0,p),sig.substr(p+1));
  }

  signal_spec(std::string name, std::string id) : user_name_(name), identifier_(id)
  {
    type_  = id.substr(id.find('-')+1);
    // assume format is correct
    if (type_.empty())
    {
      is_any_type_ = true;
      return;
    }
    if (type_[0] != '<')
    {
      return;
    }
    is_vector_     = true;
    type_          = type_.substr(1, type_.length() - 2);
    auto comma_pos = type_.find(',');
    auto size      = type_.substr(comma_pos + 1);
    type_      = type_.substr(0, comma_pos - 1);
    if (size.empty())
    {
      is_any_vector_= true;
      return;
    }
    if (std::isdigit(size[0]))
      size_ = std::stol(size);
    else
      user_parameter_ = size;
  }

  auto full_name() const { return user_name_ + ":" + identifier_; }
  //auto id_specifier() const { return identifier_ + "-" + type_; }
  auto user_name() const { return user_name_ ; }
  auto identifier() const { return identifier_ ; }
  auto type() const { return  type_; }
};
}   // namespace life
