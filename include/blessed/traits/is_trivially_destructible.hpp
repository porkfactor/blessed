#ifndef BLESSED_STD_TYPE_TRAITS_IS_TRIVIALLY_DESTRUCTIBLE_HPP
#define BLESSED_STD_TYPE_TRAITS_IS_TRIVIALLY_DESTRUCTIBLE_HPP

#if !defined(BLESSED_HAS_STD_IS_TRIVIALLY_DESTRUCTIBLE)

#include <blessed/blessed.hpp>

#include <type_traits>

namespace blessed
{

#if defined(BLESSED_HAS_BUILTIN_IS_TRIVIALLY_DESTRUCTIBLE)
    template <typename T>
    struct is_trivially_destructible : std::integral_constant<bool, __is_trivially_destructible(T)>
    {
    };
#else
    template <typename T>
    struct is_trivially_destructible : std::integral_constant<bool, __has_trivial_destructor(T) && std::is_destructible<T>::value>
    {
    };
#endif

} // namespace blessed

#else

#include <type_traits>

namespace blessed
{
    using ::std::is_trivially_destructible;
}

#endif
#endif
