#ifndef BLESSED_TRAITS_BOOL_CONSTANT_HPP
#define BLESSED_TRAITS_BOOL_CONSTANT_HPP

#if !(defined(__cpp_lib_bool_constant) && __cpp_lib_bool_constant >= 201505L)

#include <type_traits>

namespace blessed
{
    template <bool B>
    using bool_constant = std::integral_constant<bool, B>;
}

#else
#include <type_traits>

namespace blessed
{
    using ::std::bool_constant;
}

#endif
#endif
