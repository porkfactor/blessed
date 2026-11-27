#ifndef BLESSED_HASH_HPP
#define BLESSED_HASH_HPP

#include <blessed/blessed.hpp>
#include <blessed/bit.hpp>
#include <blessed/type_traits.hpp>
// #include <blessed/utility.hpp>

#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

namespace blessed
{

    namespace hash_detail
    {

        // Implementation of MurmurHash32 https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
        class MurmurHash
        {
        public:
            using return_type = std::size_t;

            // TODO shouldn't this take a span?
            inline return_type operator()(const void *value, std::size_t len) const noexcept;

        private:
            template <typename Size>
            static Size loadword(const void *p)
            {
                Size r;
                std::memcpy(&r, p, sizeof(r));
                return r;
            }

            // TODO using blessed shift function?
            static uint32_t rotl32(uint32_t x, int8_t r)
            {
                return (x << r) | (x >> (32 - r));
            }

            static uint32_t fmix32(uint32_t h)
            {
                h ^= h >> 16;
                h *= 0x85ebca6b;
                h ^= h >> 13;
                h *= 0xc2b2ae35;
                h ^= h >> 16;

                return h;
            }
        };

        std::size_t MurmurHash::operator()(const void *value, std::size_t len) const noexcept
        {
            // TODO make this constexpr compatible
            const auto *data = reinterpret_cast<const std::uint8_t *>(value);
            const int nblocks = static_cast<int>(len / 4);

            uint32_t h1 = 0;

            const uint32_t c1 = 0xcc9e2d51;
            const uint32_t c2 = 0x1b873593;

            const auto *blocks = reinterpret_cast<const uint32_t *>(data + nblocks * 4);

            // this for loop is weird
            for (int i = -nblocks; i != 0; ++i)
            {
                const auto *block = reinterpret_cast<const std::uint8_t *>(&blocks[i]);
                uint32_t k1;
                memcpy(&k1, block, sizeof(uint32_t));

                k1 *= c1;
                k1 = rotl32(k1, 15);
                k1 *= c2;

                h1 ^= k1;
                h1 = rotl32(h1, 13);
                h1 = (h1 * 5) + 0xe6546b64;
            }

            const uint8_t *tail = data + nblocks * 4;

            uint32_t k1 = 0;

            switch (len & 3)
            {
            case 3:
                k1 ^= static_cast<uint32_t>(tail[2] << 16);
                BLESSED_FALLTHROUGH;
            case 2:
                k1 ^= static_cast<uint32_t>(tail[1] << 8);
                BLESSED_FALLTHROUGH;
            case 1:
                k1 ^= tail[0];
                k1 *= c1;
                k1 = rotl32(k1, 15);
                k1 *= c2;
                h1 ^= k1;
            }

            h1 ^= static_cast<uint32_t>(len);

            return fmix32(h1);
        }

        // Implementation of CityHash64 https://github.com/google/cityhash/blob/master/src/city.cc
        class CityHash
        {
        public:
            using return_type = std::size_t;

            // TODO shouldn't this take a span?
            inline return_type operator()(const void *value, std::size_t len) const noexcept;

        private:
            using uint128 = std::pair<uint64_t, uint64_t>;

            // Some primes between 2^63 and 2^64.
            static constexpr uint64_t k0 = 0xc3a5c85c97cb3127ULL;
            static constexpr uint64_t k1 = 0xb492b66fbe98f273ULL;
            static constexpr uint64_t k2 = 0x9ae16a3b2f90404fULL;
            static constexpr uint64_t k3 = 0xc949d7c7509e6557ULL;

            template <typename Size>
            static Size loadword(const void *p)
            {
                Size r;
                std::memcpy(&r, p, sizeof(r));
                return r;
            }

            // TODO should these be using blessed::rotl/rotr?
            static uint64_t rotate_by_at_least_1(uint64_t val, int shift)
            {
                return (val >> shift) | (val << (64 - shift));
            }

            static uint64_t rotate(uint64_t val, int shift)
            {
                return shift == 0 ? val : ((val >> shift) | (val << (64 - shift)));
            }

            static uint64_t shift_mix(uint64_t val)
            {
                return val ^ (val >> 47);
            }

            static uint64_t hash_128_to_64(const uint128 &x)
            {
                // Murmur-inspired hashing.
                const uint64_t kMul = 0x9ddfea08eb382d69ULL;
                uint64_t a = (x.first ^ x.second) * kMul;
                a ^= (a >> 47);
                uint64_t b = (x.second ^ a) * kMul;
                b ^= (b >> 47);
                b *= kMul;
                return b;
            }

            static uint64_t hash_len_16(uint64_t u, uint64_t v)
            {
                return hash_128_to_64(uint128(u, v));
            }

            static uint64_t hash_len_16(uint64_t u, uint64_t v, uint64_t mul)
            {
                uint64_t a = (u ^ v) * mul;
                a ^= (a >> 47);
                uint64_t b = (v ^ a) * mul;
                b ^= (b >> 47);
                b *= mul;
                return b;
            }

            static uint64_t hash_len_0_to_16(const std::uint8_t *val, std::size_t len)
            {
                if (len >= 8)
                {
                    const uint64_t a = loadword<uint64_t>(val) + k2;
                    const auto b = loadword<uint64_t>(val + len - 8);
                    const uint64_t mul = k2 + len * 2;
                    const uint64_t c = rotate_by_at_least_1(b, 37) * mul + a;
                    const uint64_t d = (rotate_by_at_least_1(a, 25) + b) * mul;
                    return hash_len_16(c, d, mul);
                }

                if (len >= 4)
                {
                    const uint64_t mul = k2 + len * 2;
                    const uint64_t a = loadword<uint32_t>(val);
                    const auto b = loadword<uint32_t>(val + len - 4);
                    return hash_len_16(len + (a << 3), b, mul);
                }

                if (len > 0)
                {
                    const std::uint8_t a = val[0];
                    const std::uint8_t b = val[len >> 1];
                    const std::uint8_t c = val[len - 1];
                    const uint32_t y = static_cast<uint32_t>(a) + (static_cast<uint32_t>(b) << 8);
                    const auto z = static_cast<uint32_t>(len + (static_cast<uint32_t>(c) << 2));
                    return shift_mix(y * k2 ^ z * k0) * k2;
                }

                return k2;
            }

            static uint64_t hash_len_17_to_32(const std::uint8_t *val, std::size_t len)
            {
                const uint64_t mul = k2 + len * 2;
                const uint64_t a = loadword<uint64_t>(val) * k1;
                const auto b = loadword<uint64_t>(val + 8);
                const uint64_t c = loadword<uint64_t>(val + len - 8) * mul;
                const uint64_t d = loadword<uint64_t>(val + len - 16) * k2;

                return hash_len_16(rotate(a + b, 43) + rotate(c, 30) + d, a + rotate(b + k2, 18) + c, mul);
            }

            static std::pair<uint64_t, uint64_t>
            weak_hash_len_32_with_seeds(uint64_t w, uint64_t x, uint64_t y, uint64_t z, uint64_t a, uint64_t b)
            {
                a += w;
                b = rotate(b + a + z, 21);
                const uint64_t c = a;
                a += x;
                a += y;
                b += rotate(a, 44);
                return std::make_pair(a + z, b + c);
            }

            static std::pair<uint64_t, uint64_t> weak_hash_len_32_with_seeds(const std::uint8_t *val, uint64_t a, uint64_t b)
            {
                return weak_hash_len_32_with_seeds(
                    loadword<uint64_t>(val), loadword<uint64_t>(val + 8), loadword<uint64_t>(val + 16), loadword<uint64_t>(val + 24), a,
                    b);
            }

            static uint64_t hash_len_33_to_64(const std::uint8_t *val, size_t len)
            {
                const uint64_t mul = k2 + len * 2;
                uint64_t a = loadword<uint64_t>(val) * k2;
                auto b = loadword<uint64_t>(val + 8);
                const auto c = loadword<uint64_t>(val + len - 24);
                const auto d = loadword<uint64_t>(val + len - 32);
                const uint64_t e = loadword<uint64_t>(val + 16) * k2;
                const uint64_t f = loadword<uint64_t>(val + 24) * 9;
                const auto g = loadword<uint64_t>(val + len - 8);
                const uint64_t h = loadword<uint64_t>(val + len - 16) * mul;
                const uint64_t u = rotate(a + g, 43) + (rotate(b, 30) + c) * 9;
                const uint64_t v = ((a + g) ^ d) + f + 1;
                const uint64_t w = blessed::byteswap((u + v) * mul) + h;
                const uint64_t x = rotate(e + f, 42) + c;
                const uint64_t y = (blessed::byteswap((v + w) * mul) + g) * mul;
                const uint64_t z = e + f + c;
                a = blessed::byteswap((x + z) * mul + y) + b;
                b = shift_mix((z + a) * mul + d + h) * mul;
                return b + x;
            }
        };

        std::size_t CityHash::operator()(const void *value, std::size_t len) const noexcept
        {
            const auto *val = reinterpret_cast<const std::uint8_t *>(value);

            if (len <= 32)
            {
                if (len <= 16)
                {
                    return static_cast<std::size_t>(hash_len_0_to_16(val, len));
                }

                return static_cast<std::size_t>(hash_len_17_to_32(val, len));
            }

            if (len <= 64)
            {
                return static_cast<std::size_t>(hash_len_33_to_64(val, len));
            }

            // For values over 64 bytes we hash the end first, and then as we
            // loop we keep 56 bytes of state: v, w, x, y, and z.
            auto x = loadword<uint64_t>(val + len - 40);
            uint64_t y = loadword<uint64_t>(val + len - 16) + loadword<uint64_t>(val + len - 56);
            uint64_t z = hash_len_16(loadword<uint64_t>(val + len - 48) + len, loadword<uint64_t>(val + len - 24));
            std::pair<uint64_t, uint64_t> v = weak_hash_len_32_with_seeds(val + len - 64, len, z);
            std::pair<uint64_t, uint64_t> w = weak_hash_len_32_with_seeds(val + len - 32, y + k1, x);
            x = x * k1 + loadword<uint64_t>(val);

            // Decrease len to the nearest multiple of 64, and operate on 64-byte chunks.
            len = (len - 1) & ~static_cast<decltype(len)>(63);
            do
            {
                x = rotate(x + y + v.first + loadword<uint64_t>(val + 8), 37) * k1;
                y = rotate(y + v.second + loadword<uint64_t>(val + 48), 42) * k1;
                x ^= w.second;
                y += v.first + loadword<uint64_t>(val + 40);
                z = rotate(z + w.first, 33) * k1;
                v = weak_hash_len_32_with_seeds(val, v.second * k1, x + w.first);
                w = weak_hash_len_32_with_seeds(val + 32, z + w.second, y + loadword<uint64_t>(val + 16));
                std::swap(z, x);
                val += 64;
                len -= 64;
            } while (len != 0);

            return static_cast<std::size_t>(
                hash_len_16(hash_len_16(v.first, w.first) + shift_mix(y) * k1 + z, hash_len_16(v.second, w.second) + x));
        }

        template <typename T, bool = std::is_enum<T>::value>
        struct hash_enum
        {
        };

        template <typename T>
        struct hash_enum<T, true>
        {
            template <typename Hasher = blessed::hash_detail::CityHash>
            std::size_t operator()(T val) const noexcept
            {
                auto const v = blessed::to_underlying(val);
                return Hasher{}(&v, sizeof(v));
            }
        };

    } // namespace hash_detail

    template <typename T>
    struct hash;

    template<>
    struct hash<bool>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(bool val) const noexcept
        {
            return Hasher{}(&val, sizeof(val));
        }
    };

    template<>
    struct hash<char>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(char val) const noexcept
        {
            return Hasher{}(&val, sizeof(val));
        }
    };

    template<>
    struct hash<signed char>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(signed char val) const noexcept
        {
            return Hasher{}(&val, sizeof(val));
        }
    };

    template<>
    struct hash<unsigned char>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(unsigned char val) const noexcept
        {
            return Hasher{}(&val, sizeof(val));
        }
    };

    template<>
    struct hash<wchar_t>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(wchar_t val) const noexcept
        {
            return Hasher{}(&val, sizeof(val));
        }
    };

    template<>
    struct hash<char16_t>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(char16_t val) const noexcept
        {
            return Hasher{}(&val, sizeof(val));
        }
    };

    template<>
    struct hash<char32_t>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(char32_t val) const noexcept
        {
            return Hasher{}(&val, sizeof(val));
        }
    };

    template<>
    struct hash<short>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(short val) const noexcept
        {
            return Hasher{}(&val, sizeof(val));
        }
    };

    template<>
    struct hash<unsigned short>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(unsigned short val) const noexcept
        {
            return Hasher{}(&val, sizeof(val));
        }
    };

    template<>
    struct hash<int>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(int val) const noexcept
        {
            return Hasher{}(&val, sizeof(val));
        }
    };

    template<>
    struct hash<unsigned int>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(unsigned int val) const noexcept
        {
            return Hasher{}(&val, sizeof(val));
        }
    };

    template<>
    struct hash<long>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(long val) const noexcept
        {
            return Hasher{}(&val, sizeof(val));
        }
    };

    template<>
    struct hash<unsigned long>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(unsigned long val) const noexcept
        {
            return Hasher{}(&val, sizeof(val));
        }
    };

    template<>
    struct hash<long long>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(long long val) const noexcept
        {
            return Hasher{}(&val, sizeof(val));
        }
    };

    template<>
    struct hash<unsigned long long>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(unsigned long long val) const noexcept
        {
            return Hasher{}(&val, sizeof(val));
        }
    };

    template <>
    struct hash<float>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(float val) const noexcept
        {
            if (val > 0.0F || val < -0.0F)
            {
                return Hasher{}(&val, sizeof(val));
            }

            val = std::signbit(val) ? -val : val;

            return Hasher{}(&val, sizeof(val));
        }
    };

    template <>
    struct hash<double>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(double val) const noexcept
        {
            if (val > 0.0 || val < -0.0)
            {
                return Hasher{}(&val, sizeof(val));
            }

            val = std::signbit(val) ? -val : val;

            return Hasher{}(&val, sizeof(val));
        }
    };

    template <typename T>
    struct hash : public hash_detail::hash_enum<T>
    {
    };

    template <>
    struct hash<std::nullptr_t>
    {
        std::size_t operator()(std::nullptr_t) const noexcept
        {
            return 0UL;
        }
    };

    template <typename T>
    struct hash<T *>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(const T *p) const noexcept
        {
            return Hasher{}(&p, sizeof(p));
        }
    };

    template <typename T, std::size_t N>
    struct hash<std::array<T, N>>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(const std::array<T, N> &val) const noexcept
        {
            return Hasher{}(val.data(), val.size() * sizeof(typename std::array<T, N>::value_type));
        }
    };

    template <typename T>
    struct hash<std::vector<T>>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(const std::vector<T> &val) const noexcept
        {
            return Hasher{}(val.data(), val.size() * sizeof(typename std::vector<T>::value_type));
        }
    };

    template <>
    struct hash<std::string>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(const std::string &val) const noexcept
        {
            return Hasher{}(val.data(), val.size() * sizeof(typename std::string::value_type));
        }
    };

    template <typename Rep, typename Period>
    struct hash<std::chrono::duration<Rep, Period>>
    {
        template <typename Hasher = blessed::hash_detail::CityHash>
        std::size_t operator()(const std::chrono::duration<Rep, Period> &val) const noexcept
        {
            auto count = val.count();

            return Hasher{}(&count, sizeof(count));
        }
    };

    template <template <typename> class Hasher = blessed::hash>
    class hash_combiner
    {
    public:
        static void combine(std::size_t & /*seed*/) {}

        template <typename T, typename... Rest>
        static void combine(std::size_t &seed, T &&v, Rest &&...rest)
        {
            using hash_type = Hasher<blessed::remove_cvref_t<T>>;

            seed ^= hash_type{}(std::forward<T>(v)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            combine(seed, std::forward<Rest>(rest)...);
        }
    };

    // NOTE: Deprecated, prefer hash_combiner class above, this will be removed in the next major version
    BLESSED_DEPRECATED inline void hash_combine(std::size_t & /*seed*/) {}

    template <typename T, template <typename> class Hasher = std::hash, typename... Rest>
    BLESSED_DEPRECATED void hash_combine(std::size_t &seed, T &&v, Rest &&...rest)
    {
        using hash_type = Hasher<blessed::remove_cvref_t<T>>;

        seed ^= hash_type{}(std::forward<T>(v)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        hash_combine(seed, std::forward<Rest>(rest)...);
    }

}

#endif
