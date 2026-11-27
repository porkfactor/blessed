#ifndef BLESSED_FUNCTIONAL_HPP_
#define BLESSED_FUNCTIONAL_HPP_

#if !(defined(__cpp_lib_invoke) && __cpp_lib_invoke >= 201411L)

#include <blessed/blessed.hpp>
#include <blessed/traits/enable_if_t.hpp>

#include <functional>
#include <utility>

namespace blessed
{

    template <typename Fn, typename... Args, typename = blessed::enable_if_t<std::is_member_pointer<decay_t<Fn>>::value>, int = 0>
    constexpr auto invoke(Fn &&f, Args &&...args) noexcept(noexcept(std::mem_fn(f)(std::forward<Args>(args)...)))
        -> decltype(std::mem_fn(f)(std::forward<Args>(args)...))
    {
        return std::mem_fn(f)(std::forward<Args>(args)...);
    }

    template <typename Fn, typename... Args, typename = blessed::enable_if_t<!std::is_member_pointer<decay_t<Fn>>{}>>
    constexpr auto invoke(Fn &&f, Args &&...args) noexcept(noexcept(std::forward<Fn>(f)(std::forward<Args>(args)...)))
        -> decltype(std::forward<Fn>(f)(std::forward<Args>(args)...))
    {
        return std::forward<Fn>(f)(std::forward<Args>(args)...);
    }
}

#else

#include <functional>

namespace blessed
{
    using ::std::invoke;
}

#endif

#endif
