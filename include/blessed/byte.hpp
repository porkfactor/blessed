#ifndef BLESSED_BYTE_HPP_
#define BLESSED_BYTE_HPP_

#include <cstddef>

#include <blessed/blessed.hpp>
#include <blessed/type_traits.hpp>

#if BLESSED_CXX17 || (defined(__cpp_lib_byte) && __cpp_lib_byte >= 201603)
namespace blessed
{
    using byte = std::byte;
}
#else
namespace blessed
{
    enum class byte : unsigned char {};

    template <class IntegerType, class = typename blessed::enable_if_t<std::is_integral<IntegerType>::value>>
    inline BLESSED_CXX14_CONSTEXPR byte &operator<<=(byte &b, IntegerType shift) noexcept
    {
        return b = b << shift;
    }

    template <class IntegerType, class = typename blessed::enable_if_t<std::is_integral<IntegerType>::value>>
    inline BLESSED_CONSTEXPR byte operator<<(byte b, IntegerType shift) noexcept
    {
        return byte(static_cast<blessed::underlying_type_t<byte>>(b) << shift);
    }

    template <class IntegerType, class = typename blessed::enable_if_t<std::is_integral<IntegerType>::value>>
    inline BLESSED_CXX14_CONSTEXPR byte &operator>>=(byte &b, IntegerType shift) noexcept
    {
        return b = b >> shift;
    }

    template <class IntegerType, class = typename blessed::enable_if_t<std::is_integral<IntegerType>::value>>
    inline BLESSED_CONSTEXPR byte operator>>(byte b, IntegerType shift) noexcept
    {
        return byte(static_cast<blessed::underlying_type_t<byte>>(b) >> shift);
    }

    inline BLESSED_CXX14_CONSTEXPR byte &operator|=(byte &l, byte r) noexcept
    {
        return l = byte(static_cast<blessed::underlying_type_t<byte>>(l) | static_cast<blessed::underlying_type_t<byte>>(r));
    }

    inline BLESSED_CONSTEXPR byte operator|(byte l, byte r) noexcept
    {
        return byte(static_cast<blessed::underlying_type_t<byte>>(l) | static_cast<blessed::underlying_type_t<byte>>(r));
    }

    inline BLESSED_CXX14_CONSTEXPR byte &operator&=(byte &l, byte r) noexcept
    {
        return l = byte(static_cast<blessed::underlying_type_t<byte>>(l) & static_cast<blessed::underlying_type_t<byte>>(r));
    }

    inline BLESSED_CONSTEXPR byte operator&(byte l, byte r) noexcept
    {
        return byte(static_cast<blessed::underlying_type_t<byte>>(l) & static_cast<blessed::underlying_type_t<byte>>(r));
    }

    inline BLESSED_CXX14_CONSTEXPR byte &operator^=(byte &l, byte r) noexcept
    {
        return l = byte(static_cast<blessed::underlying_type_t<byte>>(l) ^ static_cast<blessed::underlying_type_t<byte>>(r));
    }

    inline BLESSED_CONSTEXPR byte operator^(byte l, byte r) noexcept
    {
        return byte(static_cast<blessed::underlying_type_t<byte>>(l) ^ static_cast<blessed::underlying_type_t<byte>>(r));
    }

    inline BLESSED_CONSTEXPR byte operator~(byte b) noexcept
    {
        return byte(~static_cast<blessed::underlying_type_t<byte>>(b));
    }

    template <class IntegerType, class = typename std::enable_if<std::is_integral<IntegerType>::value>::type>
    inline BLESSED_CONSTEXPR IntegerType to_integer(byte b) noexcept
    {
        return static_cast<IntegerType>(b);
    }

    template <bool E, typename T>
    inline BLESSED_CONSTEXPR byte to_byte_impl(T t) noexcept
    {
        static_assert(E, "blessed::to_byte(t) must be provided an unsigned char, otherwise data loss may occur. ");
        return static_cast<byte>(t);
    }
    template <>
    inline BLESSED_CONSTEXPR byte to_byte_impl<true, unsigned char>(unsigned char t) noexcept
    {
        return byte(t);
    }

    template <typename T>
    inline BLESSED_CONSTEXPR byte to_byte(T t) noexcept
    {
        return to_byte_impl<blessed::is_same<T, unsigned char>::value, T>(t);
    }

    template <int I>
    inline BLESSED_CONSTEXPR byte to_byte() noexcept
    {
        static_assert(I >= 0 && I <= 255, "blessed::byte only has 8 bits of storage, values must be in range 0-255");
        return static_cast<byte>(I);
    }
}
#endif

#endif
