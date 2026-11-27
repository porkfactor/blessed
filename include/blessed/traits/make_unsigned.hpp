#ifndef BLESSED_TRAITS_MAKE_UNSIGNED_HPP
#define BLESSED_TRAITS_MAKE_UNSIGNED_HPP

#if __cplusplus < 201304L

#include <type_traits>

namespace blessed
{
    template <typename T>
    using make_unsigned_t = typename ::std::make_unsigned<T>::type;
}

#else

#include <type_traits>

namespace blessed
{

    using ::std::make_unsigned_t;

}

#endif

#endif
