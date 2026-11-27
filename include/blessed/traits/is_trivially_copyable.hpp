
#ifndef BLESSED_STD_TYPE_TRAITS_IS_TRIVIALLY_COPYABLE_HPP
#define BLESSED_STD_TYPE_TRAITS_IS_TRIVIALLY_COPYABLE_HPP

#if !defined(BLESSED_HAS_STD_IS_TRIVIALLY_COPYABLE)

#include <blessed/blessed.hpp>
#include <blessed/traits/transformation_trait_aliases.hpp>

namespace blessed
{

template<typename T>
struct is_trivially_copyable : std::integral_constant<bool, std::is_trivial<T>::value>
{
};

}    // namespace blessed

#else

#include <type_traits>

namespace blessed
{

using ::std::is_trivially_copyable;

}

#endif

#endif
