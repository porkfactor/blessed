
#ifndef BLESSED_STD_TYPE_TRAITS_IS_SWAPPABLE_HPP
#define BLESSED_STD_TYPE_TRAITS_IS_SWAPPABLE_HPP

#if defined __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif

#if !(defined(__cpp_lib_is_swappable) && __cpp_lib_is_swappable >= 201603L)

#include <algorithm>

#include <blessed/blessed.hpp>
#include <blessed/traits/is_same.hpp>
#include <blessed/traits/is_move_constructible.hpp>
#include <blessed/traits/is_move_assignable.hpp>
#include <blessed/traits/enable_if_t.hpp>
#include <blessed/traits/void_t.hpp>

namespace blessed
{
    namespace detail
    {
        template<typename T>
        using swap_result_t = blessed::enable_if_t<blessed::is_move_constructible<T>::value && blessed::is_move_assignable<T>::value>;

        template <typename T, typename U, typename = void>
        inline const bool is_swappable_with_v = false;

        template <typename T>
        inline const bool is_swappable_v = is_swappable_with_v<T&, T&>;

        template <typename T, typename U, bool = is_swappable_with_v<T, U>>
        inline const bool is_nothrow_swappable_with_v = false;

        template <typename T>
        inline const bool is_nothrow_swappable_v = is_nothrow_swappable_with_v<T&, T&>;

        template <typename T>
        inline BLESSED_CXX20_CONSTEXPR swap_result_t<T> swap(T& __x, T& __y) noexcept(blessed::is_nothrow_move_constructible<T>::value&& blessed::is_nothrow_move_assignable<T>::value);

        template <typename T, size_t _Np, blessed::enable_if_t<is_swappable_v<T>, int> = 0>
        inline BLESSED_CXX20_CONSTEXPR void swap(T (&__a)[_Np], T (&__b)[_Np]) noexcept(is_nothrow_swappable_v<T>);

        template <typename T, typename U>
        inline const bool is_swappable_with_v<T, U,
            blessed::void_t<decltype(swap(std::declval<T>(), std::declval<U>())), decltype(swap(std::declval<U>(), std::declval<T>()))> > = true;
    }

    template <typename T, typename U>
    inline constexpr bool is_swappable_with_v = detail::is_swappable_with_v<T, U>;

    template <typename T, typename U>
    struct is_swappable_with : bool_constant<is_swappable_with_v<T, U>> {};
}

#else

#include <type_traits>

namespace blessed
{
    using ::std::is_nothrow_swappable;
    using ::std::is_nothrow_swappable_with;
    using ::std::is_swappable;
    using ::std::is_swappable_with;
}

#endif
#endif
