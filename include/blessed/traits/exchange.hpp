#ifndef BLESSED_STD_UTILITY_EXCHANGE_HPP
#define BLESSED_STD_UTILITY_EXCHANGE_HPP

#if defined __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif

#if !(defined(__cpp_lib_exchange_function) && __cpp_lib_exchange_function >= 201304L)

#include <blessed/traits/transformation_trait_aliases.hpp>

namespace blessed
{
    template <typename T, typename U = T>
    BLESSED_CXX14_CONSTEXPR T exchange(T &obj, U &&new_value)
    {
        T old_value = std::move(obj);
        obj = std::forward<U>(new_value);
        return old_value;
    }
}

#else

#include <utility>

namespace blessed
{
    using ::std::exchange;
}

#endif
#endif
