#ifndef BLESSED_TRAITS_ADD_VOLATILE_T_HPP_
#define BLESSED_TRAITS_ADD_VOLATILE_T_HPP_

#include <type_traits>

#if !(defined(__cpp_lib_transformation_trait_aliases) && __cpp_lib_transformation_trait_aliases >= 201304L)

namespace blessed
{
    template <typename T>
    using add_volatile_t = typename std::add_volatile<T>::type;
}

#else

namespace blessed
{
    using ::std::add_volatile_t;
}

#endif
#endif
