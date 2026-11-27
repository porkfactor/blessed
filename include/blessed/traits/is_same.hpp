#ifndef BLESSED_TRAITS_IS_SAME_HPP_
#define BLESSED_TRAITS_IS_SAME_HPP_

#include <blessed/traits/bool_constant.hpp>

namespace blessed
{
    template <class _Tp, class _Up>
    struct is_same : blessed::bool_constant<__is_same(_Tp, _Up)> {};
}

#endif
