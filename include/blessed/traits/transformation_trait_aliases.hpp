
#ifndef BLESSED_TRAITS_TRANSFORMATION_TRAIT_ALIASES_HPP
#define BLESSED_TRAITS_TRANSFORMATION_TRAIT_ALIASES_HPP

#include <blessed/traits/add_const_t.hpp>
#include <blessed/traits/add_cv_t.hpp>
#include <blessed/traits/add_lvalue_reference_t.hpp>
#include <blessed/traits/add_pointer_t.hpp>
#include <blessed/traits/add_rvalue_reference_t.hpp>
#include <blessed/traits/add_volatile_t.hpp>
#include <blessed/traits/enable_if_t.hpp>
#include <blessed/traits/remove_const_t.hpp>

#if !(defined(__cpp_lib_transformation_trait_aliases) && __cpp_lib_transformation_trait_aliases >= 201304L)

#include <type_traits>

namespace blessed
{
    template <typename T>
    using remove_cv_t = typename std::remove_cv<T>::type;

    template <typename T>
    using remove_volatile_t = typename std::remove_volatile<T>::type;

    template <typename T>
    using underlying_type_t = typename std::underlying_type<T>::type;

    template <typename T>
    using remove_reference_t = typename std::remove_reference<T>::type;

    template <typename T>
    using make_signed_t = typename std::make_signed<T>::type;

    template <typename T>
    using make_unsigned_t = typename std::make_unsigned<T>::type;

    template <typename T>
    using remove_extent_t = typename std::remove_extent<T>::type;

    template <typename T>
    using remove_all_extents_t = typename std::remove_all_extents<T>::type;

    template <typename T>
    using remove_pointer_t = typename std::remove_pointer<T>::type;

    template <typename T>
    using decay_t = typename std::decay<T>::type;

    template <bool b, typename T, typename F>
    using conditional_t = typename std::conditional<b, T, F>::type;

    template <typename... Ts>
    using common_type_t = typename std::common_type<Ts...>::type;

    template <typename T>
    using underlying_type_t = typename std::underlying_type<T>::type;

    template <typename T>
    using result_of_t = typename std::result_of<T>::type;

} // namespace blessed

#else

#include <type_traits>

namespace blessed
{
    using ::std::aligned_storage_t;
    using ::std::aligned_union_t;
    using ::std::common_type_t;
    using ::std::conditional_t;
    using ::std::decay_t;
    using ::std::make_signed_t;
    using ::std::make_unsigned_t;
    using ::std::remove_all_extents_t;
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
