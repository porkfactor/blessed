#ifndef BLESSED_TRAITS_ADD_CONST_T_HPP_
#define BLESSED_TRAITS_ADD_CONST_T_HPP_

#include <type_traits>

#if !(defined(__cpp_lib_transformation_trait_aliases) && __cpp_lib_transformation_trait_aliases >= 201304L)

namespace blessed
{
    template <typename T>
    using add_const_t = typename std::add_const<T>::type;
}

#else

namespace blessed
{
    using ::std::add_const_t;
}

#endif
#endif
