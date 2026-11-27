#ifndef BLESSED_ALGORITHM_HPP
#define BLESSED_ALGORITHM_HPP

#include <blessed/blessed.hpp>

#include <cstddef>
#include <cstdint>

namespace blessed
{
    namespace algo
    {
        namespace detail
        {
            template <typename T>
            struct alignpow2
            {
            };

            template <typename T>
            BLESSED_CONSTEXPR T shift_val(T val, std::size_t shift)
            {
                return static_cast<T>(val | val >> shift);
            }

            template <>
            struct alignpow2<std::uint8_t>
            {
                BLESSED_CONSTEXPR std::uint8_t operator()(std::uint8_t val) const
                {
                    return static_cast<std::uint8_t>(shift_val(shift_val(shift_val(static_cast<std::uint8_t>(val - 1U), 1U), 2U), 4U) + 1U);
                }
            };

            template <>
            struct alignpow2<std::uint16_t>
            {
                BLESSED_CONSTEXPR std::uint16_t operator()(std::uint16_t val) const
                {
                    return static_cast<std::uint16_t>(
                        shift_val(shift_val(shift_val(shift_val(static_cast<std::uint16_t>(val - 1U), 1U), 2U), 4U), 8U) + 1U);
                }
            };

            template <>
            struct alignpow2<std::uint32_t>
            {
                BLESSED_CONSTEXPR std::uint32_t operator()(std::uint32_t val) const
                {
                    return static_cast<std::uint32_t>(
                        shift_val(shift_val(shift_val(shift_val(shift_val(static_cast<std::uint32_t>(val - 1U), 1U), 2U), 4U), 8U), 16U) + 1);
                }
            };

            template <>
            struct alignpow2<std::uint64_t>
            {
                BLESSED_CONSTEXPR std::uint64_t operator()(std::uint64_t val) const
                {
                    return static_cast<std::uint64_t>(
                        shift_val(
                            shift_val(shift_val(shift_val(shift_val(shift_val(static_cast<std::uint64_t>(val - 1U), 1U), 2U), 4U), 8U), 16U),
                            32U) +
                        1U);
                }
            };

        } // namespace detail

        template <typename T>
        BLESSED_CONSTEXPR T alignpow2(T val)
        {
            return detail::alignpow2<T>()(val);
        }

        template <typename T>
        BLESSED_CONSTEXPR bool is_power_of_2(T val)
        {
            return val && !(val & (val - 1));
        }

        template <typename T, typename U>
        BLESSED_CONSTEXPR T align_floor(T val, U align)
        {
            return static_cast<T>(static_cast<uintptr_t>(val) & ~(static_cast<uintptr_t>(align) - 1));
        }

        template <typename T, typename U>
        BLESSED_CONSTEXPR T align_ceil(T val, U align)
        {
            return align_floor(val + align - 1, align);
        }
    }
}

#endif
