#ifndef BLESSED_TRAITS_IS_SIGNED_V_HPP_
#define BLESSED_TRAITS_IS_SIGNED_V_HPP_

#if !BLESSED_CXX17

namespace blessed
{
    template<typename T>
    using is_signed_v = std::is_signed<T>::value;
}

#else

namespace blessed
{
    using ::std::is_signed_v;
}

#endif

#endif
