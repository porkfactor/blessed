#ifndef BLESSED_STD_UTILITY_TO_UNDERLYING_HPP
#define BLESSED_STD_UTILITY_TO_UNDERLYING_HPP

#if defined __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif

#if !(defined(__cpp_lib_to_underlying) && __cpp_lib_to_underlying >= 202102L)

#include <blessed/traits/transformation_trait_aliases.hpp>

namespace blessed
{
    template <typename Enum>
    constexpr underlying_type_t<Enum> to_underlying(Enum e) noexcept
    {
        return static_cast<underlying_type_t<Enum>>(e);
    }
}

#else

#include <utility>

namespace blessed
{
    using ::std::to_underlying;
}

#endif
#endif
