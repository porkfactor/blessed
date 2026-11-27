#ifndef BLESSED_STD_DETAIL_IS_INVOCABLE_HPP
#define BLESSED_STD_DETAIL_IS_INVOCABLE_HPP

#if defined __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif

#if !(defined(__cpp_lib_is_invocable) && __cpp_lib_is_invocable >= 201703L)

#include <blessed/functional.hpp>

namespace blessed
{

namespace detail
{

template<typename F, typename, typename... Us>
struct invoke_result_impl;

template<typename F, typename... Us>
struct invoke_result_impl<F, decltype(blessed::invoke(std::declval<F>(), std::declval<Us>()...), void()), Us...>
{
    using type = decltype(blessed::invoke(std::declval<F>(), std::declval<Us>()...));
};

}    // namespace detail

template<typename F, typename... Us>
using invoke_result = detail::invoke_result_impl<F, void, Us...>;

template<typename F, typename... Us>
using invoke_result_t = typename invoke_result<F, Us...>::type;

namespace detail
{

template<typename, typename R, typename F, typename... Args>
struct is_invocable_r_impl : std::false_type
{
};

template<typename R, typename F, typename... Args>
struct is_invocable_r_impl<typename std::is_convertible<blessed::invoke_result_t<F, Args...>, R>::type, R, F, Args...>
        : std::true_type
{
};

}    // namespace detail

template<typename R, typename F, typename... Args>
using is_invocable_r = detail::is_invocable_r_impl<std::true_type, R, F, Args...>;

}    // namespace blessed

#else

#include <type_traits>

namespace blessed
{
    using ::std::invoke_result;
    using ::std::invoke_result_t;
    using ::std::is_invocable_r;
}

#endif

#endif
