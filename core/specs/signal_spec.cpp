
#include <string>
#include <vector>
#include <iostream>

#include "signal_spec.hpp"
#include "configuration_primitive.hpp"
#include "../utilities/utilities.hpp"

namespace ded
{
namespace specs
{

	struct SpecError;

bool
    SignalSpec::exactlyMatches(SignalSpec s)
{
  return is_any_type_ == s.is_any_type_ && is_vector_ == s.is_vector_ &&
         vector_type_ == s.vector_type_ &&
         is_any_vector_size_ == s.is_any_vector_size_ && size_ == s.size_;
}

bool
    SignalSpec::convertibleTo(SignalSpec other)
{
  if (other.is_any_type_)
    return true;
  if (!is_vector_)
  {
    if (other.is_vector_)
      return false;
    return full_type_ == other.full_type_;
  }
  if (!other.is_vector_ || vector_type_ != other.vector_type_)
    return false;
  if (other.is_any_vector_size_)
    return true;
  return size_ == other.size_;
}

SignalSpec::SignalSpec(std::string sig)
{
  auto p = sig.find(':');
  *this  = SignalSpec(sig.substr(0, p), sig.substr(p + 1));
}

SignalSpec::SignalSpec(std::string name, std::string idt)
{
  auto p = idt.find('-');
  *this  = SignalSpec(name, idt.substr(0, p), idt.substr(p + 1));
}

SignalSpec::SignalSpec(std::string name, std::string id, std::string type)
    : user_name_(name), additional_identifier_(id), full_type_(type)
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
  vector_type_   = full_type_.substr(1, comma_pos - 1);
  auto size      = full_type_.substr(comma_pos + 1);
  size.pop_back();
  if (size.empty())
  {
    is_any_vector_size_ = true;
    return;
  }
  if (std::isdigit(size[0]))
    size_ = std::stol(size);
  else
  {
    is_user_set_vector_size_ = true;
    user_parameter_          = size;
  }
}

}   // namespace specs
}   // namespace ded
