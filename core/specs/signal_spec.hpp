
#pragma once

#include <string>
#include <vector>

#include "../utilities/term_colours.hpp"

namespace ded
{
namespace specs
{

class SignalSpec
{
private:
  std::string user_name_;
  std::string additional_identifier_;
  std::string full_type_;

  std::string vector_type_;
  std::string user_parameter_;
  long        size_ = 0;
  bool is_any_type_ = false, is_vector_ = false, is_any_vector_size_ = false,
       is_user_set_vector_size_ = false, is_explicitly_bound_ = false;

public:
  SignalSpec() = default;

  SignalSpec(std::string sig);

  SignalSpec(std::string name, std::string idt);

  SignalSpec(std::string name, std::string id, std::string type);

  auto
      fullName() const
  {
    return user_name_ + ":" + additional_identifier_ + "-" + full_type_;
  }

  auto
      diagnosticName() const
  {
    return user_name_ + " : " + full_type_ +
           (is_user_set_vector_size_
                ? utilities::TermColours::blue_fg + "    with [" +
                      user_parameter_ + " = " + std::to_string(size_) + "]" +
                      utilities::TermColours::reset
                : "");
  }

  auto
      userName() const
  {
    return user_name_;
  }
  auto
      identifier() const
  {
    return additional_identifier_;
  }
  auto
      type() const
  {
    return full_type_;
  }
  auto
      userParameter() const
  {
    return user_parameter_;
  }

  void
      updateIdentifier(std::string id)
  {
    additional_identifier_ = id;
  }

  void
      instantiateUserParameter(long size)
  {
    size_ = size;
  }

  bool exactlyMatches(SignalSpec);
  bool convertibleTo(SignalSpec);
  bool isExplicitlyBound() { return is_explicitly_bound_; }
  void setExplicitlyBound() { is_explicitly_bound_ = true; }
};

std::pair<std::string, SignalSpec>
    toSignal(std::pair<std::string, std::string> user_spec);

using SignalSpecSet = 
  std::vector<std::pair<std::string, SignalSpec>>;

// struct io_signals
struct IO
{
	public :
  SignalSpecSet inputs_;
  SignalSpecSet outputs_;
//  void bindTo(SubstrateSpec, std::string);
};

struct Tags
{
	public :
  SignalSpecSet pre_;
  SignalSpecSet post_;
};
}   // namespace specs
}   // namespace ded