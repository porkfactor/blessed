#ifndef BLESSED_TRAITS_MAKE_REVERSE_ITERATOR_HPP
#define BLESSED_TRAITS_MAKE_REVERSE_ITERATOR_HPP

#if !(defined(__cpp_lib_make_reverse_iterator) && __cpp_lib_make_reverse_iterator >= 201402L)

#include <blessed/blessed.hpp>

#include <iterator>

namespace blessed
{
    template <typename Iter>
    BLESSED_CONSTEXPR std::reverse_iterator<Iter> make_reverse_iterator(Iter i)
    {
        return std::reverse_iterator<Iter>(i);
    }
}

#else

#include <iterator>

namespace blessed
{
    using ::std::make_reverse_iterator;
}

#endif
#endif
