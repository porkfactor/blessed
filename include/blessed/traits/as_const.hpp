#ifndef BLESSED_TRAITS_AS_CONST_HPP
#define BLESSED_TRAITS_AS_CONST_HPP

#if defined __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif

#if !(defined(__cpp_lib_as_const) && __cpp_lib_as_const >= 201510L)

#include <blessed/blessed.hpp>
#include <blessed/traits/add_const_t.hpp>

namespace blessed
{
    template <typename T>
    BLESSED_CXX14_CONSTEXPR blessed::add_const_t<T> &as_const(T &t) noexcept
    {
        return t;
    }

    template <typename T>
    void as_const(const T &&) = delete;
}

#else

#include <utility>

namespace blessed
{
    using ::std::as_const;
}

#endif
#endif
