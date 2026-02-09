#ifndef BLESSED_TYPE_TRAITS_HPP_
#define BLESSED_TYPE_TRAITS_HPP_

#include <type_traits>

#include <blessed/blessed.hpp>

#if BLESSED_CXX14
namespace blessed
{
    template<bool B, typename T = void>
    using enable_if_t = std::enable_if_t<B, T>;

    template<typename T>
    using remove_cv_t = std::remove_cv_t<T>;

    template<typename T>
    using remove_pointer_t = std::remove_pointer_t<T>;
}
#else
namespace blessed
{
    template<bool B, typename T = void>
    using enable_if_t = typename std::enable_if<B, T>::type;

    template<typename T>
    using remove_cv_t = typename std::remove_cv<T>::type;

    template<typename T>
    using remove_pointer_t = typename std::remove_pointer<T>::type;
}
#endif


#endif