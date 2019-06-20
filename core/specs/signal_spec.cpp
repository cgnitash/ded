
#include "signal_spec.h"

#include <string>
#include <vector>

namespace life {



  signal_spec::signal_spec(std::string sig) {
	  auto p = sig.find(':');
	  *this = signal_spec(sig.substr(0,p),sig.substr(p+1));
  }

  signal_spec::signal_spec(std::string name, std::string idt)
  {
	  auto p = idt.find('-');
	  *this = signal_spec(name, idt.substr(0,p),idt.substr(p+1));
  }

  signal_spec::signal_spec(std::string name, std::string id, std::string type)
      : user_name_(name), identifier_(id), full_type_(type)
  {
    // assume format is correct
    if (full_type_.empty())
    {
      is_any_type_ = true;
      return;
    }
    if (full_type_[0] != '<')
    {
      return;
    }
    is_vector_     = true;
    auto comma_pos = full_type_.find(',');
    vector_type_      = full_type_.substr(1, comma_pos - 1);
    auto size      = full_type_.substr(comma_pos + 1);
	size.pop_back();
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
}   // namespace life
