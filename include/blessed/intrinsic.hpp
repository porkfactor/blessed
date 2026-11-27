
#ifndef BLESSED_INTRINSIC_HPP
#define BLESSED_INTRINSIC_HPP

#include <blessed/blessed.hpp>
#include <blessed/type_traits.hpp>

#include <limits>

namespace blessed
{

namespace intrinsic
{

template<typename T, typename blessed::enable_if_t<(sizeof(T) <= sizeof(unsigned int)), int> = 0>
static inline int ctz(T v) noexcept
{
#ifdef _MSC_VER
    unsigned long tz = 0;

    return _BitScanForward(&tz, static_cast<unsigned int>(v)) ? tz : std::numeric_limits<T>::digits;
#else
    return __builtin_ctz(static_cast<unsigned int>(v));
#endif
}

#if defined(ULONGLONG_MAX) && (ULONG_MAX != ULONGLONG_MAX)
template<
    typename T,
    typename blessed::enable_if_t<((sizeof(T) > sizeof(unsigned int)) && (sizeof(T) <= sizeof(unsigned long))), int> = 0>
static inline int ctz(T v)
{
#ifdef _MSC_VER
    unsigned long tz = 0;

    return _BitScanForward(&tz, static_cast<unsigned long>(v)) ? tz : std::numeric_limits<T>::digits;
#else
    return __builtin_ctzl(static_cast<unsigned long>(v));
#endif
}

template<
    typename T,
    typename blessed::enable_if_t<((sizeof(T) > sizeof(unsigned long)) && (sizeof(T) <= sizeof(unsigned long long))), int> = 0>
static inline int ctz(T v)
{
#ifdef _MSC_VER

#ifdef _WIN64
    unsigned long tz = 0;

    return _BitScanForward64(&tz, static_cast<unsigned long long>(v)) ? tz : std::numeric_limits<T>::digits;
#else
    /* _BitScanForward64 is not available in i686 builds */
    unsigned long tz = 0;

    if(!_BitScanForward(&tz, static_cast<uint32_t>(v)))
    {
        if(_BitScanForward(&tz, static_cast<uint32_t>((v) >> 32)))
        {
            tz += 32;
        }
        else
        {
            return std::numeric_limits<T>::digits;
        }
    }

    return tz;
#endif

#else
    return __builtin_ctzll(static_cast<unsigned long long>(v));
#endif
}

#else

template<typename T, typename blessed::enable_if_t<((sizeof(T) == sizeof(unsigned long long))), int> = 0>
static inline int ctz(T v)
{
#ifdef _MSC_VER

#ifdef _WIN64
    unsigned long tz = 0;

    return _BitScanForward64(&tz, v) ? tz : std::numeric_limits<T>::digits;
#else
    /* _BitScanForward64 is not available in i686 builds */
    unsigned long tz = 0;

    if(!_BitScanForward(&tz, static_cast<uint32_t>(v)))
    {
        if(_BitScanForward(&tz, static_cast<uint32_t>((v) >> 32)))
        {
            tz += 32;
        }
        else
        {
            return std::numeric_limits<T>::digits;
        }
    }

    return tz;
#endif

#else
    return __builtin_ctzll(v);
#endif
}
#endif

#ifdef _MSC_VER
template<typename T, typename blessed::enable_if_t<(sizeof(T) <= sizeof(unsigned long)), int> = 0>
static inline int clz(T v)
{
    unsigned long lz = 0;

    return _BitScanReverse(&lz, static_cast<unsigned long>(v)) ? (std::numeric_limits<T>::digits - 1) - lz : 0;
}

template<
    typename T,
    typename blessed::enable_if_t<((sizeof(T) > sizeof(unsigned long)) && (sizeof(T) <= sizeof(unsigned long long))), int> = 0>
static inline int clz(T v)
{
    unsigned long lz = 0;

    return _BitScanReverse64(&lz, static_cast<unsigned long long>(v)) ? (std::numeric_limits<T>::digits - 1) - lz : 0;
}
#else
template<typename T, typename blessed::enable_if_t<(sizeof(T) <= sizeof(unsigned int)), int> = 0>
static inline int clz(T v)
{
    return __builtin_clz(static_cast<unsigned int>(v)) -
           (std::numeric_limits<unsigned int>::digits - std::numeric_limits<T>::digits);
}

#if defined(ULONGLONG_MAX) && (ULONG_MAX != ULONGLONG_MAX)
template<
    typename T,
    typename blessed::enable_if_t<((sizeof(T) > sizeof(unsigned int)) && (sizeof(T) <= sizeof(unsigned long))), int> = 0>
static inline int clz(T v)
{
    return __builtin_clzl(static_cast<unsigned long>(v)) -
           (std::numeric_limits<unsigned long>::digits - std::numeric_limits<T>::digits);
}

template<
    typename T,
    typename blessed::enable_if_t<((sizeof(T) > sizeof(unsigned long)) && (sizeof(T) <= sizeof(unsigned long long))), int> = 0>
static inline int clz(T v)
{
    return __builtin_clzll(static_cast<unsigned long long>(v)) -
           (std::numeric_limits<unsigned long long>::digits - std::numeric_limits<T>::digits);
}
#else
template<typename T, typename blessed::enable_if_t<((sizeof(T) == sizeof(unsigned long long))), int> = 0>
static inline int clz(T v)
{
    return __builtin_clzll(v) - (std::numeric_limits<unsigned long long>::digits - std::numeric_limits<T>::digits);
}
#endif

#endif

#ifdef _MSC_VER
template<typename T, typename blessed::enable_if_t<(sizeof(T) <= sizeof(unsigned int)), int> = 0>
static inline int popcount(T v)
{
    return static_cast<int>(__popcnt(static_cast<unsigned int>(v)));
}
template<
    typename T,
    typename blessed::enable_if_t<((sizeof(T) > sizeof(unsigned int)) && (sizeof(T) <= sizeof(unsigned long long))), int> = 0>
static inline int popcount(T v)
{
    return static_cast<int>(__popcnt64(static_cast<unsigned long long>(v)));
}
#else
template<typename T, typename blessed::enable_if_t<(sizeof(T) <= sizeof(unsigned int)), int> = 0>
static inline int popcount(T v)
{
    return __builtin_popcount(static_cast<unsigned int>(v));
}

#if defined(ULONGLONG_MAX) && (ULONG_MAX != ULONGLONG_MAX)
template<
    typename T,
    typename blessed::enable_if_t<((sizeof(T) > sizeof(unsigned int)) && (sizeof(T) <= sizeof(unsigned long))), int> = 0>
static inline int popcount(T v)
{
    return __builtin_popcountl(static_cast<unsigned long>(v));
}

template<
    typename T,
    typename blessed::enable_if_t<((sizeof(T) > sizeof(unsigned long)) && (sizeof(T) <= sizeof(unsigned long long))), int> = 0>
static inline int popcount(T v)
{
    return __builtin_popcountll(static_cast<unsigned long long>(v));
}
#else
template<typename T, typename blessed::enable_if_t<(sizeof(T) == sizeof(unsigned long long)), int> = 0>
static inline int popcount(T v)
{
    return __builtin_popcountll(v);
}
#endif

#endif

}    // namespace intrinsic

}    // namespace blessed

#endif
