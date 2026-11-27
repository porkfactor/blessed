#ifndef BLESSED_STD_ITERATOR_SSIZE_HPP
#define BLESSED_STD_ITERATOR_SSIZE_HPP

#if defined __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif

#if !(defined(__cpp_lib_ssize) && __cpp_lib_ssize >= 201902L)

#include <blessed/blessed.hpp>

#include <cstddef>
#include <initializer_list>
#include <iterator>

namespace blessed
{

    template <typename C>
    BLESSED_CXX14_CONSTEXPR auto ssize(const C &c) -> blessed::common_type_t<std::ptrdiff_t, blessed::make_signed_t<decltype(c.size())>>
    {
        using R = blessed::common_type_t<std::ptrdiff_t, blessed::make_signed_t<decltype(c.size())>>;

        return static_cast<R>(c.size());
    }

    template <typename T, std::ptrdiff_t N>
    BLESSED_CXX14_CONSTEXPR std::ptrdiff_t ssize(const T (& /* unused */)[N]) noexcept
    {
        return N;
    }

} // namespace blessed

#else

#include <iterator>

namespace blessed
{

    using ::std::ssize;

}

#endif
#endif
