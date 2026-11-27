#ifndef BLESSED_STD_TYPE_TRAITS_REMOVE_CVREF_HPP
#define BLESSED_STD_TYPE_TRAITS_REMOVE_CVREF_HPP

#if defined __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif

#if !(defined(__cpp_lib_remove_cvref) && __cpp_lib_remove_cvref >= 201711L)

#include <blessed/traits/transformation_trait_aliases.hpp>

namespace blessed
{
    template <typename T>
    struct remove_cvref
    {
        using type = remove_cv_t<remove_reference_t<T>>;
    };

    template <typename T>
    using remove_cvref_t = typename remove_cvref<T>::type;
}

#else

#include <type_traits>

namespace blessed
{
    using ::std::remove_cvref;
    using ::std::remove_cvref_t;
}

#endif
#endif
