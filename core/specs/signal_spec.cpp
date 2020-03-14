
#include <iostream>
#include <string>
#include <vector>
#include<range/v3/action/remove.hpp>

#include "../utilities/utilities.hpp"
#include "configuration_primitive.hpp"
#include "signal_spec.hpp"

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
    SignalSpec::sliceableBy(long from, long to, long every, std::string &vtt)
{
  if (!is_vector_)
    return false;
  if (size_ < to)
    return false;
  size_ = (to - from) / every;
  vtt = vector_type_ ;

  return true;
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

SignalSpec::SignalSpec(std::string type) : full_type_(type)
{

  std::smatch m;
  if (!std::regex_match(full_type_, m, valid_signal_type_))
  {
    std::cout << "unable to parse signal type " << full_type_ << "\n";
    throw SpecError{};
  }

  if (m[0].str().empty())
  {
    is_any_type_ = true;
    return;
  }

  if (auto non_vector_type = m[1].str(); !non_vector_type.empty())
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
  {
    size_ = std::stol(size);
    bindings_.indices_ = rv::iota(0, size_) | rs::to<std::vector<int>>;
  }
  else
  {
    is_user_set_vector_size_ = true;
    user_parameter_          = size;
  }
}

void
    SignalSpec::instantiateUserParameter(long size)
{
  size_              = size;
  bindings_.indices_ = rv::iota(0, size_) | rs::to<std::vector<int>>;
}

void
    SignalSpec::updatePlaceholders(SignalSpec in)
{

  if (full_type_ == "_")
    full_type_ = in.full_type_;
  if (vector_type_ == "_")
    vector_type_ = in.vector_type_;
  if (is_vector_)
    size_ = in.size_;
}

std::string
    SignalSpec::diagnosticName() const
{
  return full_type_ +
         (is_user_set_vector_size_ || is_placeholder_vector_size_
              ? utilities::TermColours::blue_fg + "    with [ " +
                    (user_parameter_.empty() ? "_" : user_parameter_) + " = " +
                    std::to_string(size_) + " ]" + utilities::TermColours::reset
              : "");
  }

void
    SignalSpec::addBoundIndices(std::vector<int> indices)
{
  for (auto i : indices)
     bindings_.indices_ |= ra::remove(i);

  bindings_.is_bound_ |= bindings_.indices_.empty();
}

NamedSignal
    toSignal(SignalConstraint user_constraint)
{
  return { user_constraint.name_, SignalSpec{ user_constraint.type_ } };
}

}   // namespace specs
}   // namespace ded
