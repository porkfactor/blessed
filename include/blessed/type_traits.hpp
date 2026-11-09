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

    template<typename T>
    using add_const_t = std::add_const_t<T>;

    template<typename T>
    using add_pointer_t = std::add_pointer_t<T>;

    template<typename T, typename U>
    using is_same = std::is_same<T, U>;
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

    template<typename T>
    using add_const_t = typename std::add_const<T>::type;

    template<typename T>
    using add_pointer_t = typename std::add_pointer<T>::type;

    template<typename T, typename U>
    struct is_same : std::false_type {};

    template<typename T>
    struct is_same<T, T> : std::true_type {};
}
#endif


#endif