#ifndef BLESSED_BLESSED_HPP_
#define BLESSED_BLESSED_HPP_

#ifdef __GNUC__
    #define BLESSED_LIKELY(x)           __builtin_expect(!!(x), 1)
    #define BLESSED_UNLIKELY(x)         __builtin_expect(!!(x), 0)
    #define BLESSED_ATTRIBUTE(x)        __attribute__((x))

    #define BLESSED_BIG_ENDIAN          (__BYTE_ORDER__ __ORDER_BIG_ENDIAN__)
    #define BLESSED_LITTLE_ENDIAN       (__BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__)
#else
    #define BLESSED_LIKELY(x)           (x)
    #define BLESSED_UNLIKELY(x)         (x)
    #define BLESSED_ATTRIBUTE(x)

    #ifdef _MSC_VER
        #if defined(_M_X64) || defined(_M_IX86)
            #define BLESSED_LITTLE_ENDIAN 1
            #define BLESSED_BIG_ENDIAN    0
        #else
            #define BLESSED_LITTLE_ENDIAN 0
            #define BLESSED_BIG_ENDIAN    1
        #endif
    #endif
#endif

#define BLESSED_CXX11_VERSION           201103L
#define BLESSED_CXX14_VERSION           201303L
#define BLESSED_CXX17_VERSION           201703L
#define BLESSED_CXX20_VERSION           202002L
#define BLESSED_CXX23_VERSION           202302L

#define BLESSED_CXX23                   (__cplusplus >= BLESSED_CXX23_VERSION)
#define BLESSED_CXX20                   (__cplusplus >= BLESSED_CXX20_VERSION)
#define BLESSED_CXX17                   (__cplusplus >= BLESSED_CXX17_VERSION)
#define BLESSED_CXX14                   (__cplusplus >= BLESSED_CXX14_VERSION)
#define BLESSED_CXX11                   (__cplusplus >= BLESSED_CXX11_VERSION)

#define BLESSED_HIDE_FROM_ABI

#if defined(__cpp_constexpr)
#define BLESSED_CONSTEXPR_VALUE constexpr
#else
#define BLESSED_CONSTEXPR_VALUE const
#endif

#if defined(__cpp_constexpr) && __cpp_constexpr >= 200704L
#define BLESSED_CONSTEXPR constexpr
#else
#define BLESSED_CONSTEXPR
#endif

#if BLESSED_CXX14 || (defined(__cpp_constexpr) && __cpp_constexpr >= 201304)
#define BLESSED_CXX14_CONSTEXPR constexpr
#else
#define BLESSED_CXX14_CONSTEXPR
#endif

#if BLESSED_CXX20 || (defined(__cpp_constexpr) && __cpp_constexpr >= 202002)
#define BLESSED_CXX20_CONSTEXPR constexpr
#else
#define BLESSED_CXX20_CONSTEXPR
#endif

#if defined(__has_cpp_attribute)
#define BLESSED_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define BLESSED_HAS_CPP_ATTRIBUTE(x) (0)
#endif

#if BLESSED_HAS_CPP_ATTRIBUTE(nodiscard)
#define BLESSED_NODISCARD [[nodiscard]]
#elif BLESSED_HAS_CPP_ATTRIBUTE(gnu::warn_unused_result)
#define BLESSED_NODISCARD [[gnu::warn_unused_result]]
#else
#define BLESSED_NODISCARD BLESSED_ATTRIBUTE(warn_unused_result)
#endif

#if BLESSED_HAS_CPP_ATTRIBUTE(maybe_unused)
#define BLESSED_UNUSED [[maybe_unused]]
#else
#define BLESSED_UNUSED BLESSED_ATTRIBUTE(unused)
#endif

#if BLESSED_HAS_CPP_ATTRIBUTE(no_unique_address)
#define BLESSED_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
#define BLESSED_NO_UNIQUE_ADDRESS
#endif

#if BLESSED_HAS_CPP_ATTRIBUTE(fallthrough)
#define BLESSED_FALLTHROUGH [[fallthrough]]
#elif BLESSED_HAS_CPP_ATTRIBUTE(gnu::fallthrough)
#define BLESSED_FALLTHROUGH [[gnu::fallthrough]]
#else
#define BLESSED_FALLTHROUGH /* fall-through */
#endif

#if BLESSED_CXX14 && BLESSED_HAS_CPP_ATTRIBUTE(deprecated)
#define BLESSED_DEPRECATED [[deprecated]]
#elif BLESSED_HAS_CPP_ATTRIBUTE(gnu::deprecated)
#define BLESSED_DEPRECATED [[gnu::deprecated]]
#else
#define BLESSED_DEPRECATED BLESSED_ATTRIBUTE(deprecated)
#endif

#define BLESSED_FORCE_INLINE BLESSED_ATTRIBUTE(always_inline) inline
#define BLESSED_NEVER_INLINE BLESSED_ATTRIBUTE(noinline) 

#if(__has_feature(cxx_exceptions))
#define BLESSED_INTERNAL_TRY            try
#define BLESSED_INTERNAL_CATCH_ANY      catch(...)
#define BLESSED_INTERNAL_RETHROW        \
    do                                  \
    {                                   \
        throw;                          \
    } while(false)
#else
#define BLESSED_INTERNAL_TRY            if(true)
#define BLESSED_INTERNAL_CATCH_ANY      else if(false)
#define BLESSED_INTERNAL_RETHROW        \
    do                                  \
    {                                   \
    } while(false)
#endif


#endif
