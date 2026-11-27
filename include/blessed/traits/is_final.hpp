#ifndef BLESSED_TRAITS_IS_FINAL_HPP
#define BLESSED_TRAITS_IS_FINAL_HPP

#if !(defined(__cpp_lib_is_final) && __cpp_lib_is_final >= 201402L)

#include <type_traits>

namespace blessed
{
    template <typename T>
    struct is_final : public std::false_type
    {
    };

}

#else

#include <type_traits>

namespace blessed
{

    using ::std::is_final;

}

#endif

#endif
