
// a collection of template meta-programs

#pragma once

#include <type_traits>

namespace ded
{
namespace utilities
{
namespace TMP
{

// Primary template handles all types not supporting the operation.
template <template <typename...> typename Call,
          typename Return,
          typename AlwaysVoid,
          typename... T>
struct is_detected : std::false_type
{
};

// // Specialization recognizes/validates only types supporting the archetype.
template <template <typename...> typename Call, typename Return, typename... T>
struct is_detected<Call, Return, std::void_t<Call<T...>>, T...>
    : std::is_same<Call<T...>, Return>
{
};

template <typename T, typename Return, template <typename...> typename Call>
struct has_signature : is_detected<Call, Return, void, T>
{
};

// dependent template to allow for static_assert error messages
template <typename = void>
struct concept_fail : std::false_type
{
};

// template class that contains an overload set of operator()
template <typename... lambdas>
struct overload_set : lambdas...
{
  using lambdas::operator()...;
};

// Deduction guide to instantiate the template
template <typename... lambdas>
overload_set(lambdas...)->overload_set<lambdas...>;

}   // namespace TMP
}   // namespace utilities
}   // namespace ded
