#ifndef BLESSED_STD_TYPE_TRAITS_LOGICAL_TRAITS_HPP
#define BLESSED_STD_TYPE_TRAITS_LOGICAL_TRAITS_HPP

#if defined __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif

#if !(defined(__cpp_lib_logical_traits) && __cpp_lib_logical_traits >= 201510L)

#include <blessed/traits/transformation_trait_aliases.hpp>

namespace blessed
{

    template <typename...>
    struct disjunction : std::false_type
    {};

    template <typename B1>
    struct disjunction<B1> : B1
    {};

    template <typename B1, typename... Bn>
    struct disjunction<B1, Bn...> : conditional_t<bool(B1::value), B1, disjunction<Bn...>>
    {};

    template <typename... Ts>
    struct conjunction : std::true_type
    {};

    template <typename T, typename... Ts>
    struct conjunction<T, Ts...> : conditional_t<T::value, conjunction<Ts...>, T>
    {};

    template <typename T>
    struct conjunction<T> : T
    {};

    template <typename T>
    struct negation : blessed::bool_constant<!bool(T::value)>
    {};
}

#else

#include <type_traits>

namespace blessed
{
    using ::std::conjunction;
    using ::std::disjunction;
    using ::std::negation;
}

#endif

#endif
