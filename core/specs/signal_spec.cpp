
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

	// forward declaration
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
    return full_type_ == other.full_type_ || other.full_type_ == "_";
  }
  if (!other.is_vector_ ||
      (other.vector_type_ != "_" && vector_type_ != other.vector_type_))
    return false;
  if (other.is_any_vector_size_ || other.is_placeholder_vector_size_)
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
    : user_name_(name), internal_identifier_(id), full_type_(type)
{

  std::smatch m;
 if (! std::regex_match(full_type_,m, valid_signal_type_))
 {
	 std::cout << "unable to parse signal type " << full_type_ << "\n";
		 throw SpecError{};
 }
  
 if (m[0].str().empty())
 {
	 is_any_type_ = true;
	 return;
 }
 
 
 if (auto non_vector_type = m[1].str();
		 !non_vector_type.empty())
 {
	 full_type_ = non_vector_type;
	 return;
 }

 is_vector_   = true;
 vector_type_ = m[2].str();

 if (auto size = m[3].str(); size.empty())
   is_any_vector_size_ = true;
 else if (size == "_")

   is_placeholder_vector_size_ = true;

 else if (std::isdigit(size[0]))
   size_ = std::stol(size);
 else
 {
   is_user_set_vector_size_ = true;
   user_parameter_          = size;
 }
}

  void SignalSpec::updatePlaceholders(SignalSpec in){
		  
	  if (full_type_ == "_")
		  full_type_ = in.full_type_;
	  if (vector_type_ == "_")
		  vector_type_ = in.full_type_;
	  if (is_placeholder_vector_size_)
		  size_ = in.size_;
  
  }

NamedSignal
    toSignal(SignalConstraint user_constraint)
{
  auto name  = user_constraint.name_;
  auto type =  user_constraint.type_;
  return { name, SignalSpec{ name, name, type } };
}

std::ostream &
    operator<<(std::ostream &out, TagType const &t)
{
  if (t == TagType::pre)
	out << "pre";
  else
	out << "post";
  return out;
}

}   // namespace specs
}   // namespace ded
