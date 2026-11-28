#ifndef BLESSED_STATIC_BITSET_HPP_
#define BLESSED_STATIC_BITSET_HPP_

#include <algorithm>

#include <blessed/blessed.hpp>
#include <blessed/bit.hpp>
#include <blessed/type_traits.hpp>

#include <bitset>

namespace blessed
{
    namespace detail
    {
        template<typename T>
        class bit_reference
        {
        public:
            using value_type = T;
            using pointer = blessed::add_pointer_t<T>;

            bit_reference(pointer p, value_type m) :
                ref_(p),
                mask_(m)
            {}

            bit_reference(bit_reference const &other) noexcept :
                ref_(other.ref_),
                mask_(other.mask_)
            {}

            bit_reference &operator=(bool v) noexcept
            {
                return set(v.test());
            }

            bit_reference &operator=(bool v) const noexcept
            {
                return set(v.test());
            }

            bit_reference &operator=(bit_reference const &other)
            {
                return *this;
            }

            operator bool() const noexcept
            {
                return test();
            }

            bool operator~() const noexcept
            {
                return !test();
            }

            bit_reference &flip(void)
            {
                (*ref_) ^= mask_;
                return *this;
            }

        private:
            inline bool test(void) const noexcept
            {
                return ((*ref_) & mask_) != value_type{};
            }

            inline void set(bool v) const noexcept
            {
                if (v)
                {
                    *ref_ |= mask_;
                }
                else
                {
                    *ref_ &= ~mask_;
                }
            }

            pointer ref_;
            value_type mask_;
        };

        template<typename T>
        class bit_iterator
        {

        };
    }

    template<std::size_t N>
    class static_bitset
    {
    public:
        using storage_type = unsigned long;

        using reference = detail::bit_reference<storage_type>;

        static_bitset()
        {
            reset();
        }

        static_bitset(static_bitset const &other) :
            data_(other.data_)
        {}

        bool any(void) const noexcept
        {
            for (auto const v : data_)
            {
                if (v != value_type{})
                    return true;
            }

            return false;
        }

        bool all(void) const noexcept
        {
            for (auto const v : data_)
            {
                if (v != value_type{-1})
                {
                    return false;
                }
            }

            return true;
        }

        bool none(void) const noexcept
        {
            for (auto const v : data_)
            {
                if (v != value_type{})
                    return false;
            }

            return true;
        }

        bool test(std::size_t pos) const
        {
            return operator[](pos);
        }

        std::size_t count(void) const noexcept
        {
            return popcount();
        }

        std::size_t size(void) const noexcept
        {
            return N;
        }

        static_bitset &set(void)
        {
            return *this;
        }

        static_bitset &set(std::size_t pos, bool v = true)
        {
            return *this;
        }

        static_bitset &reset(void)
        {
            return set();
        }

        static_bitset &reset(std::size_t pos)
        {
            return set(pos, false);
        }

        static_bitset &flip(void)
        {
            return *this;
        }

        static_bitset &flip(std::size_t pos)
        {
            return *this;
        }

        static_bitset &operator=(static_bitset const &other)
        {
            data_ = other.data_;
            return *this;
        }

        static_bitset &operator&=(static_bitset &other)
        {
            return *this;
        }

        bool operator[](std::size_t) const
        {
            return false;
        }

        reference operator[](std::size_t)
        {
            return reference{nullptr, 0};
        }

        static_bitset &operator&=(static_bitset const &other)
        {
            std::transform(
                other.data_.begin(),
                other.data_.end(),
                data_.begin(),
                data_.end(),
                [](value_type const &l, value_type const &r)
                {
                    return l & r;
                });

            return *this;
        }

        friend inline static_bitset operator&(static_bitset const &l, static_bitset const &r)
        {
            static_bitset result{l};
            return result &= r;
        }

        static_bitset &operator|=(static_bitset const &other)
        {
            std::transform(
                other.data_.begin(),
                other.data_.end(),
                data_.begin(),
                data_.end(),
                [](value_type const &l, value_type const &r)
                {
                    return l | r;
                });

            return *this;
        }

        friend inline static_bitset operator|(static_bitset const &l, static_bitset const &r)
        {
            static_bitset result{l};
            return result |= r;
        }

        static_bitset &operator^=(static_bitset const &other)
        {
            for (std::size_t idx = 0; idx < data_.size(); ++idx)
            {
                // TODO: clipping mask
                data_[idx] = (data_[idx] ^ other.data_[idx]);
            }

            return *this;
        }

        friend inline static_bitset operator^(static_bitset const &l, static_bitset const &r)
        {
            static_bitset result{l};
            return result ^= r;
        }

        static_bitset &operator<<=(std::size_t pos)
        {
            return *this;
        }

        friend inline bitset operator<<(static_bitset const &l, std::size_t pos)
        {
            static_bitset result{l};
            return result <<= pos;
        }

        static_bitset &operator>>=(std::size_t pos)
        {
            return *this
        }

        friend inline static_bitset operator>>(static_bitset const &l, std::size_t pos)
        {
            static_bitset result{l};
            return result >>= pos;
        }

        friend bool operator==(static_bitset const &l, static_bitset const &r)
        {
            return l.data_ == r.data_;
        }

        friend bool operator!=(static_bitset const &l, static_bitset const &r)
        {
            return l.data_ != r.data_;
        }

        void const *data() const
        {
            return data_.data();
        }

        std::size_t byte_size() const
        {
            return data_.size() * sizeof(value_type);
        }

    private:
        std::size_t popcount(void) const
        {
            std::accumulate(
                data_.begin(),
                data_.end(),
                value_type{},
                [](value_type const &a, value_type const &b)
                {
                    return a + static_cast<value_type>(blessed::popcount(b));
                });
        }

        std::size_t countl_zero(std::size_t pos = npos) const
        {
            std::size_t count{};
            return count;
        }

        std::size_t countl_one(std::size_t pos = npos) const
        {
            std::size_t count{};
            return count;
        }

        std::size_t countr_zero(std::size_t pos = 0) const
        {
            std::size_t count{};

            for (auto v : data_)
            {
                if (v != 0)
                {
                    count += blessed::countr_zero(v);
                    break;
                }

                count += bit_size();
            }

            return count;
        }

        std::size_t countr_one(std::size_t pos = 0) const
        {
            std::size_t count{};

            for  (auto v : data_)
            {
                if (v != value_type{-1})
                {
                    count += blessed::countr_one(v);
                    break;
                }

                count += bit_size();
            }
            return count;
        }

        std::array<value_type, value_count> data_;
    };
}

#endif
