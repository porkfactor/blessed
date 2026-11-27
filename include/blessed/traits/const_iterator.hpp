#ifndef BLESSED_TRAITS_CONST_ITERATOR_HPP
#define BLESSED_TRAITS_CONST_ITERATOR_HPP

#include <blessed/blessed.hpp>

#include <iterator>

namespace blessed
{
    template <typename C>
    BLESSED_CXX14_CONSTEXPR auto cbegin(const C &c) noexcept(noexcept(std::begin(c))) -> decltype(std::begin(c))
    {
        return std::begin(c);
    }

    template <typename C>
    BLESSED_CXX14_CONSTEXPR auto cend(const C &c) noexcept(noexcept(std::end(c))) -> decltype(std::end(c))
    {
        return std::end(c);
    }

    template <typename C>
    BLESSED_CXX14_CONSTEXPR auto rbegin(C &c) -> decltype(c.rbegin())
    {
        return c.rbegin();
    }

    template <typename C>
    BLESSED_CXX14_CONSTEXPR auto rbegin(const C &c) -> decltype(c.rbegin())
    {
        return c.rbegin();
    }

    template <typename T, std::size_t N>
    BLESSED_CXX14_CONSTEXPR std::reverse_iterator<T *> rbegin(T (&array)[N])
    {
        return std::reverse_iterator<T *>(array + N);
    }

    template <typename C>
    BLESSED_CXX14_CONSTEXPR auto crbegin(const C &c) noexcept(noexcept(blessed::rbegin(c))) -> decltype(blessed::rbegin(c))
    {
        return blessed::rbegin(c);
    }

    template <typename C>
    BLESSED_CXX14_CONSTEXPR auto rend(C &c) -> decltype(c.rend())
    {
        return c.rend();
    }

    template <typename C>
    BLESSED_CXX14_CONSTEXPR auto rend(const C &c) -> decltype(c.rend())
    {
        return c.rend();
    }

    template <typename T, std::size_t N>
    BLESSED_CXX14_CONSTEXPR std::reverse_iterator<T *> rend(T (&array)[N])
    {
        return std::reverse_iterator<T *>(array);
    }

    template <typename C>
    BLESSED_CXX14_CONSTEXPR auto crend(const C &c) noexcept(noexcept(blessed::rend(c))) -> decltype(blessed::rend(c))
    {
        return blessed::rend(c);
    }
}

/*
#else
namespace blessed
{
    using ::std::cbegin;
    using ::std::cend;
    using ::std::crbegin;
    using ::std::crend;
    using ::std::rbegin;
    using ::std::rend;
}

#endif
*/
#endif
