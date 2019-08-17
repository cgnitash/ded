
#include "signal_spec.h"
#include "configuration_primitive.h"
#include "../utilities/utilities.h"

#include <string>
#include <vector>
#include <iostream>

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

void IO::bindTo(IO ios)
{
  for (auto &n_sig : inputs_)
  {
    auto &in_sig  = n_sig.second;
    auto  matches = rs::count_if(ios.inputs_, [sig = in_sig](auto ns) {
      return ns.second.exactlyMatches(sig);
    });
    if (matches > 1)
    {
      std::cout << "error: multiple input signals match exactly\n";
      // throw;
    }
    if (!matches)
    {
      std::cout << "error: no input signals match exactly (convertible signals "
                   "not supported yet)\n  "
                << n_sig.second.fullName() << "\nviable candidates";
      for (auto sig : ios.inputs_)
        std::cout << "\n    " << sig.second.fullName();
      throw SpecError{};
    }
    auto i = rs::find_if(ios.inputs_, [sig = in_sig](auto ns) {
      return ns.second.exactlyMatches(sig);
    });
    in_sig.updateIdentifier(i->second.identifier());
    ios.inputs_.erase(i);
  }

  for (auto &n_sig : outputs_)
  {
    auto &out_sig = n_sig.second;
    auto  matches = rs::count_if(ios.outputs_, [sig = out_sig](auto ns) {
      return ns.second.exactlyMatches(sig);
    });
    if (matches > 1)
    {
      std::cout << "error: multiple output signals match exactly\n";
      // throw;
    }
    if (!matches)
    {
      std::cout << "error: no input signals match exactly (convertible signals "
                   "not supported yet)\n  "
                << n_sig.second.fullName() << "\nviable candidates";
      for (auto sig : ios.outputs_)
        std::cout << "\n    " << sig.second.fullName();
      throw SpecError{};
    }
    auto i = rs::find_if(ios.outputs_, [sig = out_sig](auto ns) {
      return ns.second.exactlyMatches(sig);
    });
    out_sig.updateIdentifier(i->second.identifier());
    ios.outputs_.erase(i);
  }
}
}   // namespace specs
}   // namespace ded
