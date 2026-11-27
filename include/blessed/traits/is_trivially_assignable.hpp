
#ifndef BLESSED_STD_TYPE_TRAITS_IS_TRIVIALLY_ASSIGNABLE_HPP
#define BLESSED_STD_TYPE_TRAITS_IS_TRIVIALLY_ASSIGNABLE_HPP

#if !defined(BLESSED_HAS_STD_IS_TRIVIALLY_ASSIGNABLE)

#include <blessed/blessed.hpp>
#include <blessed/traits/transformation_trait_aliases.hpp>

#include <type_traits>

namespace blessed
{

template<typename T>
struct is_trivially_move_assignable;

namespace detail
{

template<typename T>
union single_member_union
{
    T t;
};

template<typename T>
struct is_trivially_move_assignable_reference : std::false_type
{
};

template<typename T>
struct is_trivially_move_assignable_reference<T &> : ::blessed::is_trivially_move_assignable<T>::type
{
};

template<typename T>
struct is_trivially_move_assignable_reference<T &&> : ::blessed::is_trivially_move_assignable<T>::type
{
};

}    // namespace detail

template<typename T>
struct is_trivially_move_assignable
        : std::conditional<
              std::is_object<T>::value && !std::is_array<T>::value && std::is_move_assignable<T>::value,
              std::is_move_assignable<blessed::detail::single_member_union<T>>,
              blessed::detail::is_trivially_move_assignable_reference<T>>::type::type
{
};

#if defined(BLESSED_HAS_BUILTIN_IS_TRIVIALLY_ASSIGNABLE)
template<typename T>
struct is_trivially_copy_assignable : std::integral_constant<
                                          bool,
                                          __is_trivially_assignable(
                                              typename std::add_lvalue_reference<T>::type,
                                              typename std::add_lvalue_reference<typename std::add_const<T>::type>::type)>
{
};

template<typename T, typename U>
struct is_trivially_assignable : std::integral_constant<bool, __is_trivially_assignable(T, U)>
{
};
#else
template<typename T>
struct is_trivially_copy_assignable
        : std::integral_constant<
              bool,
              __has_trivial_assign(typename std::remove_reference<T>::type) && std::is_copy_assignable<T>::value>
{
};

template<typename T, typename U>
struct is_trivially_assignable : std::integral_constant<bool, __has_trivial_assign(typename std::remove_reference<T>::type)>
{
};
#endif

}    // namespace blessed

#else

#include <type_traits>

namespace blessed
{

using ::std::is_trivially_assignable;
using ::std::is_trivially_copy_assignable;
using ::std::is_trivially_move_assignable;

}    // namespace blessed

#endif

#endif
