#ifndef BLESSED_STD_TYPE_TRAITS_IS_TRIVIALLY_DEFAULT_CONSTRUCTIBLE_HPP
#define BLESSED_STD_TYPE_TRAITS_IS_TRIVIALLY_DEFAULT_CONSTRUCTIBLE_HPP

#if !defined(BLESSED_HAS_STD_IS_TRIVIALLY_DEFAULT_CONSTRUCTIBLE)

#include <blessed/blessed.hpp>
#include <blessed/traits/transformation_trait_aliases.hpp>

namespace blessed
{

#if defined(BLESSED_HAS_BUILTIN_IS_TRIVIALLY_CONSTRUCTIBLE)
template<typename T>
struct is_trivially_default_constructible : std::integral_constant<bool, __is_trivially_constructible(T)>
{
};
#else
template<typename T>
struct is_trivially_default_constructible
        : std::integral_constant<
              bool,
              __has_trivial_constructor(T) && std::is_default_constructible<T>::value && blessed::is_trivially_destructible<T>::value>
{
#if defined(BLESSED_HAS_STD_IS_TRIVIALLY_CONSTRUCTIBLE)
private:
    static constexpr bool compliant =
        std::is_trivially_default_constructible<T>::value == is_trivially_default_constructible::value;

    static_assert(
        compliant || std::is_trivially_default_constructible<T>::value,
        "Not compliant with std::is_trivially_default_constructible; Standard: false, Implementation: true");

    static_assert(
        compliant || !std::is_trivially_default_constructible<T>::value,
        "Not compliant with std::is_trivially_default_constructible; Standard: true, Implementation: false");
#endif
};
#endif

}    // namespace blessed

#else

#include <type_traits>

namespace blessed
{

using ::std::is_trivially_default_constructible;

}

#endif

#endif
