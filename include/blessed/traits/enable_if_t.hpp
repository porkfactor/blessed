#ifndef BLESSED_TRAITS_ENABLE_IF_T_HPP_
#define BLESSED_TRAITS_ENABLE_IF_T_HPP_

#include <type_traits>

#if !(defined(__cpp_lib_transformation_trait_aliases) && __cpp_lib_transformation_trait_aliases >= 201304L)

namespace blessed
{
    template <bool b, typename T = void>
    using enable_if_t = typename std::enable_if<b, T>::type;
}

#else

namespace blessed
{
    using ::std::enable_if_t;
}

#endif
#endif