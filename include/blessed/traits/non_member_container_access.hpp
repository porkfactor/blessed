#ifndef BLESSED_STD_ITERATOR_NON_MEMBER_CONTAINER_ACCESS_HPP
#define BLESSED_STD_ITERATOR_NON_MEMBER_CONTAINER_ACCESS_HPP

#if !(defined(__cpp_lib_nonmember_container_access) && __cpp_lib_nonmember_container_access >= 201411L)

#include <blessed/blessed.hpp>

#include <cstddef>
#include <initializer_list>
#include <iterator>

namespace blessed
{
    template <typename C>
    BLESSED_CONSTEXPR auto size(const C &c) -> decltype(c.size())
    {
        return c.size();
    }

    template <typename T, std::size_t N>
    BLESSED_CONSTEXPR std::size_t size(const T (& /* unused */)[N]) noexcept
    {
        return N;
    }

    template <typename C>
    BLESSED_CONSTEXPR auto empty(const C &c) -> decltype(c.empty())
    {
        return c.empty();
    }

    template <typename T, std::size_t N>
    BLESSED_CONSTEXPR bool empty(const T (& /* unused */)[N]) noexcept
    {
        return false;
    }

    template <typename E>
    BLESSED_CONSTEXPR bool empty(std::initializer_list<E> il) noexcept
    {
        return il.size() == 0;
    }

    template <typename C>
    BLESSED_CONSTEXPR auto data(C &c) -> decltype(c.data())
    {
        return c.data();
    }

    template <typename C>
    BLESSED_CONSTEXPR auto data(const C &c) -> decltype(c.data())
    {
        return c.data();
    }

    template <typename T, std::size_t N>
    BLESSED_CONSTEXPR T *data(T (&array)[N]) noexcept
    {
        return array;
    }

    template <typename E>
    BLESSED_CONSTEXPR const E *data(std::initializer_list<E> il) noexcept
    {
        return il.begin();
    }

}

#else

#include <iterator>

namespace blessed
{
    using ::std::data;
    using ::std::empty;
    using ::std::size;
}

#endif

#endif
