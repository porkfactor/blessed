#ifndef BLESSED_BLESSED_HPP_
#define BLESSED_BLESSED_HPP_

#define BLESSED_CXX11_VERSION           201103L
#define BLESSED_CXX14_VERSION           201303L
#define BLESSED_CXX17_VERSION           201703L

#define BLESSED_CXX17                   (__cplusplus >= BLESSED_CXX17_VERSION)
#define BLESSED_CXX14                   (__cplusplus >= BLESSED_CXX14_VERSION)
#define BLESSED_CXX11                   (__cplusplus >= BLESSED_CXX11_VERSION)

#define BLESSED_NODISCARD

#if BLESSED_CXX14 || (defined(__cpp_constexpr) && __cpp_constexpr >= 201304)
#define BLESSED_CXX14_CONSTEXPR constexpr
#else
#define BLESSED_CXX14_CONSTEXPR
#endif

template <class T, class U>
constexpr T narrow_cast(U&& u) noexcept
{
    return static_cast<T>(std::forward<U>(u));
}

#endif
