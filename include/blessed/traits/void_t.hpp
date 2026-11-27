#ifndef BLESSED_TRAITS_VOID_T_HPP
#define BLESSED_TRAITS_VOID_T_HPP

#if !(defined(__cpp_lib_void_t) && __cpp_lib_void_t >= 201411L)

#include <blessed/traits/transformation_trait_aliases.hpp>

namespace blessed
{
    namespace detail
    {
        template <typename... Ts>
        struct make_void
        {
            using type = void;
        };

    } // namespace detail

    template <typename... Ts>
    using void_t = typename detail::make_void<Ts...>::type;

}

#else

#include <type_traits>

namespace blessed
{
    using ::std::void_t;
}

#endif
#endif
