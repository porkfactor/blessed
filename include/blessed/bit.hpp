
#ifndef BLESSED_BIT_HPP
#define BLESSED_BIT_HPP

#if defined __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif

#include <blessed/blessed.hpp>

#if !(defined(__cpp_lib_bitops) && __cpp_lib_bitops >= 201907L)

#include <blessed/type_traits.hpp>
#include <blessed/intrinsic.hpp>

#include <cstdint>

namespace blessed
{
    namespace bitops_detail
    {
        template <typename T>
        BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR T rotl(T value, int shift)
        {
            static_assert(std::is_unsigned<T>::value, "blessed::bitops_detail::rotl may only be used on unsigned types");

            constexpr unsigned num_bits{std::numeric_limits<T>::digits};

            static_assert(num_bits && !(num_bits & (num_bits - 1)), "blessed::bitops_detail::rotl value bit length must be power of 2");

            using promoted_type = blessed::common_type_t<int, T>;
            using unsigned_promoted_type = blessed::make_unsigned_t<promoted_type>;

            constexpr signed count_mask{static_cast<signed>(num_bits - 1)};
            const signed masked_shift{shift & count_mask};

            return static_cast<T>(
                (unsigned_promoted_type{value} << masked_shift) | (unsigned_promoted_type{value} >> (-masked_shift & count_mask)));
        }

        template <typename T>
        BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR T rotr(T value, int shift)
        {
            static_assert(std::is_unsigned<T>::value, "blessed::bitops_detail::rotr may only be used on unsigned types");

            constexpr unsigned num_bits{std::numeric_limits<T>::digits};

            static_assert(num_bits && !(num_bits & (num_bits - 1)), "blessed::bitops_detail::rotr value bit length must be power of 2");

            using promoted_type = blessed::common_type_t<int, T>;
            using unsigned_promoted_type = blessed::make_unsigned_t<promoted_type>;

            constexpr signed count_mask{static_cast<signed>(num_bits - 1)};
            const signed masked_shift{shift & count_mask};

            return static_cast<T>(
                (unsigned_promoted_type{value} >> masked_shift) | (unsigned_promoted_type{value} << (-masked_shift & count_mask)));
        }

        template <typename T, typename blessed::enable_if_t<(sizeof(T) > sizeof(unsigned long long)), int> = 0>
        BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR int countr_zero(T value) noexcept
        {
            static_assert(std::is_unsigned<T>::value, "blessed::bitops_detail::countr_zero() requires an unsigned integer type");

            int ret = 0;
            constexpr unsigned int ulldigits = std::numeric_limits<unsigned long long>::digits;

            while (static_cast<unsigned long long>(value) == 0ULL)
            {
                ret += ulldigits;
                value >>= value >> ulldigits;
            }

            return ret + intrinsic::ctz(static_cast<unsigned long long>(value));
        }

        template <typename T, typename blessed::enable_if_t<(sizeof(T) <= sizeof(unsigned long long)), int> = 0>
        BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR int countr_zero(T value) noexcept
        {
            static_assert(std::is_unsigned<T>::value, "blessed::bitops_detail::countr_zero() requires an unsigned integer type");

            if (value == 0)
            {
                return std::numeric_limits<T>::digits;
            }

            return intrinsic::ctz(value);
        }

        template <typename T>
        BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR int countl_zero(T value) noexcept
        {
            static_assert(std::is_unsigned<T>::value, "blessed::bitops_detail::countl_zero() requires an unsigned integer type");

            if (value == 0)
            {
                return std::numeric_limits<T>::digits;
            }

            return intrinsic::clz(value);
        }

        template <typename T, typename blessed::enable_if_t<(sizeof(T) > sizeof(unsigned long long)), int> = 0>
        BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR int countl_zero(T value) noexcept
        {
            static_assert(std::is_unsigned<T>::value, "blessed::bitops_detail::countl_zero() requires an unsigned integer type");

            if (value == 0)
            {
                return std::numeric_limits<T>::digits;
            }

            // handle large integer types such as __uint128_t
            int ret = 0;
            int iter = 0;
            const unsigned int ulldigits = std::numeric_limits<unsigned long long>::digits;

            while (true)
            {
                value = rotr(value, ulldigits);

                if ((iter = countl_zero(static_cast<unsigned long long>(value))) != ulldigits)
                {
                    break;
                }

                ret += iter;
            }

            return ret + iter;
        }

        template <typename T>
        BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR int countl_one(T value) noexcept
        {
            static_assert(std::is_unsigned<T>::value, "blessed::bitops_detail::countl_one() requires an unsigned integer type");

            return value != std::numeric_limits<T>::max() ? countl_zero(static_cast<T>(~value)) : std::numeric_limits<T>::digits;
        }

        template <typename T>
        BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR int countr_one(T value) noexcept
        {
            static_assert(std::is_unsigned<T>::value, "blessed::bitops_detail::countr_one() requires an unsigned integer type");

            return value != std::numeric_limits<T>::max() ? countr_zero(static_cast<T>(~value)) : std::numeric_limits<T>::digits;
        }

        template <typename T, typename blessed::enable_if_t<(sizeof(T) > sizeof(unsigned long long)), int> = 0>
        BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR int popcount(T value) noexcept
        {
            int ret = 0;

            constexpr unsigned int ulldigits = std::numeric_limits<unsigned long long>::digits;

            while (value != 0)
            {
                ret += intrinsic::popcount(static_cast<unsigned long long>(value));
                value >>= ulldigits;
            }

            return ret;
        }

        template <typename T, typename blessed::enable_if_t<(sizeof(T) <= sizeof(unsigned long long)), int> = 0>
        BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR int popcount(T value) noexcept
        {
            static_assert(std::is_unsigned<T>::value, "blessed::bitops_detail::popcount() requires an unsigned integer type");

            return intrinsic::popcount(value);
        }

    } // namespace bitops_detail

    template <typename T, typename blessed::enable_if_t<std::is_unsigned<T>::value, int> = 0>
    BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR T rotl(T v, int shift) noexcept
    {
        return bitops_detail::rotl(v, shift);
    }

    template <typename T, typename blessed::enable_if_t<std::is_unsigned<T>::value, int> = 0>
    BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR T rotr(T v, int shift) noexcept
    {
        return bitops_detail::rotr(v, shift);
    }

    template <typename T, typename blessed::enable_if_t<std::is_unsigned<T>::value, int> = 0>
    BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR int popcount(T v) noexcept
    {
        return bitops_detail::popcount(v);
    }

    template <typename T, typename blessed::enable_if_t<std::is_unsigned<T>::value, int> = 0>
    BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR int countl_zero(T v) noexcept
    {
        return bitops_detail::countl_zero(v);
    }

    template <typename T, typename blessed::enable_if_t<std::is_unsigned<T>::value, int> = 0>
    BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR int countl_one(T v) noexcept
    {
        return bitops_detail::countl_one(v);
    }

    template <typename T, typename blessed::enable_if_t<std::is_unsigned<T>::value, int> = 0>
    BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR int countr_zero(T v) noexcept
    {
        return bitops_detail::countr_zero(v);
    }

    template <typename T, typename blessed::enable_if_t<std::is_unsigned<T>::value, int> = 0>
    BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR int countr_one(T v) noexcept
    {
        return bitops_detail::countr_one(v);
    }

}

#else

#include <bit>

namespace blessed
{
    using ::std::countl_one;
    using ::std::countl_zero;
    using ::std::countr_one;
    using ::std::countr_zero;
    using ::std::popcount;
    using ::std::rotl;
    using ::std::rotr;
}

#endif

#if !(defined(__cpp_lib_int_pow2) && __cpp_lib_int_pow2 >= 202002L)

#include <blessed/type_traits.hpp>

namespace blessed
{

    namespace int_pow2_detail
    {

        template <typename T>
        BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR unsigned bit_log2(T value) noexcept
        {
            static_assert(std::is_unsigned<T>::value, "blessed::int_pow2_detail::bit_log2() requires an unsigned integer type");

            return std::numeric_limits<T>::digits - 1 - blessed::bitops_detail::countl_zero(value);
        }

        template <typename T>
        BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR unsigned has_single_bit(T value) noexcept
        {
            static_assert(std::is_unsigned<T>::value, "blessed::int_pow2_detail::has_single_bit() requires an unsigned integer type");

            return value != 0 && (((value & (value - 1U)) == 0));
        }

    } // namespace int_pow2_detail

    template <typename T, typename blessed::enable_if_t<std::is_unsigned<T>::value, int> = 0>
    BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR T bit_floor(T value) noexcept
    {
        return value == 0 ? 0 : T{1} << int_pow2_detail::bit_log2(value);
    }

    template <typename T, typename blessed::enable_if_t<std::is_unsigned<T>::value, int> = 0>
    BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR T bit_ceil(T value) noexcept
    {
        if (value < 2)
        {
            return 1;
        }

        const unsigned n = std::numeric_limits<T>::digits - blessed::bitops_detail::countl_zero(static_cast<T>(value - 1U));

        assert(value != std::numeric_limits<T>::digits && "bad input to blessed::bit_ceil");

        if (sizeof(T) >= sizeof(unsigned))
        {
            return T{1} << n;
        }
        else
        {
            const unsigned extra = std::numeric_limits<unsigned>::digits - std::numeric_limits<T>::digits;
            const unsigned ret_val = 1U << (n + extra);

            return static_cast<T>(ret_val >> extra);
        }
    }

    template <typename T, typename blessed::enable_if_t<std::is_unsigned<T>::value, int> = 0>
    BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR T bit_width(T value) noexcept
    {
        return value == 0 ? 0 : int_pow2_detail::bit_log2(value) + T{1};
    }

    template <typename T, typename blessed::enable_if_t<std::is_unsigned<T>::value, int> = 0>
    BLESSED_HIDE_FROM_ABI BLESSED_CXX14_CONSTEXPR T has_single_bit(T value) noexcept
    {
        return int_pow2_detail::has_single_bit(value);
    }
}

#else

#include <bit>

namespace blessed
{
    using ::std::bit_ceil;
    using ::std::bit_floor;
    using ::std::bit_width;
    using ::std::has_single_bit;
}

#endif

#if !(defined(__cpp_lib_bit_cast) && __cpp_lib_bit_cast >= 201806L)

#include <blessed/type_traits.hpp>

#if !defined(BLESSED_HAS_BUILTIN_BIT_CAST)

#include <cstring>

namespace blessed
{
    template <class To, class From>
    BLESSED_NODISCARD blessed::enable_if_t<
        sizeof(To) == sizeof(From) && blessed::is_trivially_copyable<From>::value && blessed::is_trivially_copyable<To>::value,
        To>
    bit_cast(const From &src) noexcept
    {
        static_assert(
            blessed::is_trivially_constructible<To>::value,
            "This implementation additionally requires destination type to be trivially constructible");

        To dst;
        std::memcpy(&dst, &src, sizeof(To));
        return dst;
    }
}

#else

namespace blessed
{
    template <typename To, typename From>
    BLESSED_NODISCARD constexpr To bit_cast(const From &from) noexcept
    {
        return __builtin_bit_cast(To, from);
    }
}

#endif

#else

#include <bit>

namespace blessed
{

    using ::std::bit_cast;

}

#endif

#if !(defined(__cpp_lib_byteswap) && __cpp_lib_byteswap >= 202110L)

#include <blessed/type_traits.hpp>

#include <cstdint>

namespace blessed
{
    namespace bitops_detail
    {

        template <typename T, typename blessed::enable_if_t<(sizeof(T) == 1), int> = 0>
        constexpr T byteswap(T v)
        {
            return v;
        }

        template <typename T, typename blessed::enable_if_t<(sizeof(T) == 2), int> = 0>
        constexpr T byteswap(T v)
        {
#ifdef _WIN32
            return _byteswap_ushort(v);
#else
            return __builtin_bswap16(v);
#endif
        }

        template <typename T, typename blessed::enable_if_t<(sizeof(T) == 4), int> = 0>
        constexpr T byteswap(T v)
        {
#ifdef _WIN32
            return _byteswap_ulong(v);
#else
            return __builtin_bswap32(v);
#endif
        }

        template <typename T, typename blessed::enable_if_t<(sizeof(T) == 8), int> = 0>
        constexpr T byteswap(T v)
        {
#ifdef _WIN32
            return _byteswap_uint64(v);
#else
            return __builtin_bswap64(v);
#endif
        }

        template <typename T, typename blessed::enable_if_t<(sizeof(T) == 16), int> = 0>
        constexpr T byteswap(T v)
        {
#if defined(BLESSED_HAS_BUILTIN_BSWAP128)
            return __builtin_bswap128(v);
#else
            return static_cast<T>(byteswap(static_cast<std::uint64_t>(v))) << 64 |
                   static_cast<T>(byteswap(static_cast<std::uint64_t>(v >> 64)));
#endif
        }

        template <typename T, typename blessed::enable_if_t<(sizeof(T) > 16), int> = 0>
        constexpr T byteswap(T /*v*/)
        {
            static_assert(sizeof(T) == 0, "byteswap is unimplemented for integral types of this size");
            return {};
        }

    }

    template <typename T, typename blessed::enable_if_t<std::is_integral<T>::value, int> = 0>
    constexpr T byteswap(T v)
    {
        return bitops_detail::byteswap(v);
    }
}

#else

#include <bit>

namespace blessed
{
    using ::std::byteswap;
}

#endif

#if !(defined(__cpp_lib_endian) && __cpp_lib_endian >= 201907L)

namespace blessed
{
    enum class endian
    {
    #if defined(_MSC_VER) && !defined(__clang__)
        little = 0,
        big = 1,
        native = little,
    #else
        little = __ORDER_LITTLE_ENDIAN__,
        big = __ORDER_BIG_ENDIAN__,
        native = __BYTE_ORDER__,
    #endif
    };
}

#else

#include <bit>

namespace blessed
{
    using ::std::endian;
}

#endif

#endif
