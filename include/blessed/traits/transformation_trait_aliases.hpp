
#ifndef BLESSED_TRAITS_TRANSFORMATION_TRAIT_ALIASES_HPP
#define BLESSED_TRAITS_TRANSFORMATION_TRAIT_ALIASES_HPP

#if !(defined(__cpp_lib_transformation_trait_aliases) && __cpp_lib_transformation_trait_aliases >= 201304L)

#include <type_traits>

namespace blessed
{
    template <typename T>
    using add_const_t = typename std::add_const<T>::type;
    
    template <typename T>
    using add_cv_t = typename std::add_cv<T>::type;
    
    template <typename T>
    using add_lvalue_reference_t = typename std::add_lvalue_reference<T>::type;
    
    template <typename T>
    using add_pointer_t = typename std::add_pointer<T>::type;
    
    template <typename T>
    using add_rvalue_reference_t = typename std::add_rvalue_reference<T>::type;
    
    template <typename T>
    using add_volatile_t = typename std::add_volatile<T>::type;
    
    template <std::size_t N, std::size_t A>
    using aligned_storage_t = typename std::aligned_storage<N, A>::type;

    template <std::size_t N, typename... T>
    using aligned_union_t = typename std::aligned_union<N, T...>::type;

    template <bool b, typename T, typename F>
    using conditional_t = typename std::conditional<b, T, F>::type;

    template <typename... Ts>
    using common_type_t = typename std::common_type<Ts...>::type;

    template <typename T>
    using decay_t = typename std::decay<T>::type;

    template <typename T>
    using make_signed_t = typename std::make_signed<T>::type;

    template <typename T>
    using make_unsigned_t = typename ::std::make_unsigned<T>::type;

    template <typename T>
    using remove_all_extents_t = typename std::remove_all_extents<T>::type;

    template <typename T>
    using remove_const_t = typename std::remove_const<T>::type;

    template <typename T>
    using remove_cv_t = typename std::remove_cv<T>::type;

    template <typename T>
    using remove_extent_t = typename std::remove_extent<T>::type;

    template <typename T>
    using remove_pointer_t = typename std::remove_pointer<T>::type;

    template <typename T>
    using remove_reference_t = typename std::remove_reference<T>::type;

    template <typename T>
    using remove_volatile_t = typename std::remove_volatile<T>::type;

    template <typename T>
    using result_of_t = typename std::result_of<T>::type;

    template <typename T>
    using underlying_type_t = typename std::underlying_type<T>::type;
}

#else

#include <type_traits>

namespace blessed
{
    using ::std::aligned_storage_t;
    using ::std::aligned_union_t;
    using ::std::add_const_t;
    using ::std::add_cv_t;
    using ::std::add_lvalue_reference_t;
    using ::std::add_pointer_t;
    using ::std::add_rvalue_reference_t;
    using ::std::add_volatile_t;
    using ::std::aligned_storage_t;
    using ::std::aligned_union_t;
    using ::std::common_type_t;
    using ::std::conditional_t;
    using ::std::decay_t;
    using ::std::make_signed_t;
    using ::std::make_unsigned_t;
    using ::std::remove_all_extents_t;
    using ::std::remove_const_t;
    using ::std::remove_cv_t;
    using ::std::remove_extent_t;
    using ::std::remove_pointer_t;
    using ::std::remove_reference_t;
    using ::std::remove_volatile_t;
    using ::std::result_of_t;
    using ::std::underlying_type_t;
}
#endif

#endif
