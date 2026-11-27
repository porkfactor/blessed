#ifndef BLESSED_STD_TYPE_TRAITS_IS_TRIVIALLY_CONSTRUCTIBLE_HPP
#define BLESSED_STD_TYPE_TRAITS_IS_TRIVIALLY_CONSTRUCTIBLE_HPP

#if !defined(BLESSED_HAS_STD_IS_TRIVIALLY_CONSTRUCTIBLE)

// #include "BLESSED/base/types.h"
#include <blessed/traits/is_trivially_destructible.hpp>
#include <blessed/traits/transformation_trait_aliases.hpp>

namespace blessed
{

    namespace detail
    {
        template <typename T>
        struct is_trivially_move_constructible_object
            : std::integral_constant<
                    bool,
                    std::is_move_constructible<single_member_union<T>>::value && blessed::is_trivially_destructible<T>::value>
        {};

        template <typename T>
        struct is_trivially_copy_constructible_object
            : std::integral_constant<
                    bool,
                    std::is_copy_constructible<single_member_union<T>>::value && blessed::is_trivially_destructible<T>::value>
        {};
    }

#if defined(BLESSED_HAS_BUILTIN_IS_TRIVIALLY_CONSTRUCTIBLE)
    template <typename T, typename... Args>
    struct is_trivially_constructible : std::integral_constant<bool, __is_trivially_constructible(T)>
    {};
#else
    template <typename T, typename... Args>
    struct is_trivially_constructible
        : std::integral_constant<bool, __has_trivial_constructor(T) && std::is_constructible<T, Args...>::value>
    {};
#endif

    template <typename T>
    struct is_trivially_move_constructible : std::conditional<
                                                    std::is_object<T>::value && !std::is_array<T>::value,
                                                    detail::is_trivially_move_constructible_object<T>,
                                                    std::is_reference<T>>::type::type
    {};

    template <typename T>
    struct is_trivially_copy_constructible : std::conditional<
                                                    std::is_object<T>::value && !std::is_array<T>::value,
                                                    detail::is_trivially_copy_constructible_object<T>,
                                                    std::is_lvalue_reference<T>>::type::type
    {};
}

#else

#include <type_traits>

namespace blessed
{
    using ::std::is_trivially_constructible;
    using ::std::is_trivially_copy_constructible;
    using ::std::is_trivially_move_constructible;
}

#endif

#endif
