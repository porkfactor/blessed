#ifndef BLESSED_STRING_VIEW_HPP
#define BLESSED_STRING_VIEW_HPP

#if defined __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif

#if !(defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201603L)

#include <blessed/blessed.hpp>
#include <blessed/iterator.hpp>
#include <blessed/hash.hpp>
#include <blessed/detail/string.hpp>
#include <blessed/detail/char_traits.hpp>

#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>
#include <limits>
#include <ostream>
#include <string>
#include <type_traits>

namespace blessed
{
    template <typename CharT, typename Traits = std::char_traits<CharT>>
    class basic_string_view
    {
        const CharT *data_ = nullptr;
        std::size_t length_ = 0;

    public:
        using traits_type = Traits;
        using value_type = CharT;
        using pointer = CharT *;
        using const_pointer = const CharT *;
        using reference = CharT &;
        using const_reference = const CharT &;
        using const_iterator = const CharT *;
        using iterator = const_iterator;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using reverse_iterator = const_reverse_iterator;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        static_assert(!std::is_array<value_type>::value, "Character type of basic_string_view must not be an array");
        static_assert(std::is_standard_layout<value_type>::value, "Character type of basic_string_view must be standard-layout");
        static_assert(std::is_trivial<value_type>::value, "Character type of basic_string_view must be trivial");
        static_assert(
            std::is_same<CharT, typename traits_type::char_type>::value,
            "traits_type::char_type must be the same type as CharT");

        static const size_type npos;

        BLESSED_CXX14_CONSTEXPR basic_string_view() noexcept = default;

        BLESSED_CXX14_CONSTEXPR basic_string_view(const CharT *s, size_type count) : data_(s), length_(count)
        {
            assert(((count == 0) || (s != nullptr)) && "basic_string_view(CharT *, size_t): received nullptr");
        }

        BLESSED_CXX14_CONSTEXPR basic_string_view(const CharT *s) : data_(s), length_(traits_type::length(s)) {}

        BLESSED_CXX14_CONSTEXPR basic_string_view(const basic_string_view &other) noexcept = default;

        BLESSED_CXX14_CONSTEXPR basic_string_view(const std::basic_string<CharT, Traits> &str)
            : basic_string_view(str.data(), str.size())
        {
        }

        basic_string_view &operator=(const basic_string_view &other) noexcept = default;

        ~basic_string_view() noexcept = default;

        BLESSED_CONSTEXPR const_pointer data() const noexcept
        {
            return data_;
        }

        BLESSED_CONSTEXPR size_type length() const noexcept
        {
            return length_;
        }

        BLESSED_CONSTEXPR size_type size() const noexcept
        {
            return length();
        }

        BLESSED_CONSTEXPR size_type max_size() const noexcept
        {
            return std::numeric_limits<size_type>::max();
        }

        BLESSED_NODISCARD BLESSED_CONSTEXPR bool empty() const noexcept
        {
            return length_ == 0;
        }

        BLESSED_CXX14_CONSTEXPR const_reference operator[](size_type pos) const noexcept
        {
            assert(pos <= size() && "basic_string_view::operator[]: pos is out of range");
            return data_[pos];
        }

        BLESSED_CXX14_CONSTEXPR const_reference at(size_type pos) const
        {
#if (__has_feature(cxx_exceptions))
            if (pos <= size())
            {
                return data_[pos];
            }
            else
            {
                throw std::out_of_range("basic_string_view::at() out of range");
            }
#else
            return (*this)[pos];
#endif
        }

        BLESSED_CXX14_CONSTEXPR const_reference front() const noexcept
        {
            assert(!empty() && "basic_string_view::front(): string is empty");
            return data_[0];
        }

        BLESSED_CXX14_CONSTEXPR const_reference back() const noexcept
        {
            assert(!empty() && "basic_string_view::back(): string is empty");
            return data_[length_ - 1];
        }

        BLESSED_CXX14_CONSTEXPR const_iterator begin() const noexcept
        {
            return const_iterator(data());
        }

        BLESSED_CXX14_CONSTEXPR const_iterator cbegin() const noexcept
        {
            return const_iterator(data());
        }

        BLESSED_CXX14_CONSTEXPR const_iterator end() const noexcept
        {
            return const_iterator(data() + size());
        }

        BLESSED_CXX14_CONSTEXPR const_iterator cend() const noexcept
        {
            return const_iterator(data() + size());
        }

        BLESSED_CXX14_CONSTEXPR const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator(end());
        }

        BLESSED_CXX14_CONSTEXPR const_reverse_iterator crbegin() const noexcept
        {
            return const_reverse_iterator(cend());
        }

        BLESSED_CXX14_CONSTEXPR const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator(begin());
        }

        BLESSED_CXX14_CONSTEXPR const_reverse_iterator crend() const noexcept
        {
            return const_reverse_iterator(cbegin());
        }

        /*BLESSED_CXX14_CONSTEXPR*/ void remove_prefix(size_type n)
        {
            assert(n <= size() && "basic_string_view::remove_prefix(): can't remove more than size()");
            data_ += n;
            length_ -= n;
        }

        /*BLESSED_CXX14_CONSTEXPR*/ void remove_suffix(size_type n)
        {
            assert(n <= size() && "basic_string_view::remove_suffix(): can't remove more than size()");
            length_ -= n;
        }

        BLESSED_CXX14_CONSTEXPR void swap(basic_string_view &v) noexcept
        {
            std::swap(data_, v.data_);
            std::swap(length_, v.length_);
        }

        size_type copy(CharT *dest, size_type count, size_type pos = 0) const
        {
#if (__has_feature(cxx_exceptions))
            if (pos > size())
            {
                throw std::out_of_range("basic_string_view::copy(): pos out of range");
            }
#else
            assert(pos <= size() && "basic_string_view::copy(): pos out of range");
#endif

            size_type const rcount = std::min(count, size() - pos);

            traits_type::copy(dest, data() + pos, rcount);
            return rcount;
        }

        BLESSED_CXX14_CONSTEXPR basic_string_view substr(size_type pos = 0, size_type count = npos) const
        {
#if (__has_feature(cxx_exceptions))
            if (pos > size())
            {
                throw std::out_of_range("basic_string_view::substr(): pos out of range");
            }
#else
            assert(pos <= size() && "basic_string_view::substr(): pos out of range");
#endif

            size_type const rcount = std::min(count, size() - pos);

            return {data() + pos, rcount};
        }

        BLESSED_CXX14_CONSTEXPR int compare(basic_string_view v) const noexcept
        {
            size_type const rlen = std::min(size(), v.size());

            int sv_comp_result = traits_type::compare(data(), v.data(), rlen);

            if (sv_comp_result != 0)
            {
                return sv_comp_result;
            }

            if (size() < v.size())
            {
                return -1;
            }

            if (size() > v.size())
            {
                return 1;
            }

            return 0;
        }

        BLESSED_CXX14_CONSTEXPR int compare(size_type pos1, size_type count1, basic_string_view v) const
        {
            return substr(pos1, count1).compare(v);
        }

        BLESSED_CXX14_CONSTEXPR int
        compare(size_type pos1, size_type count1, basic_string_view v, size_type pos2, size_type count2) const
        {
            return substr(pos1, count1).compare(v.substr(pos2, count2));
        }

        BLESSED_CXX14_CONSTEXPR int compare(const CharT *s) const
        {
            return compare(basic_string_view(s));
        }

        BLESSED_CXX14_CONSTEXPR int compare(size_type pos1, size_type count1, const CharT *s) const
        {
            return substr(pos1, count1).compare(basic_string_view(s));
        }

        BLESSED_CXX14_CONSTEXPR int compare(size_type pos1, size_type count1, const CharT *s, size_type count2) const
        {
            return substr(pos1, count1).compare(basic_string_view(s, count2));
        }

#if (defined(__cpp_lib_starts_ends_with) && __cpp_lib_starts_ends_with >= 201711)
        BLESSED_CXX14_CONSTEXPR bool starts_with(basic_string_view x) const noexcept
        {
            return size() >= x.size() && compare(0, x.size(), x) == 0;
        }

        BLESSED_CXX14_CONSTEXPR bool starts_with(CharT x) const noexcept
        {
            return starts_with(basic_string_view(&x, 1));
        }

        BLESSED_CXX14_CONSTEXPR bool starts_with(const CharT *x) const
        {
            return starts_with(basic_string_view(x));
        }

        BLESSED_CXX14_CONSTEXPR bool ends_with(basic_string_view x) const noexcept
        {
            return size() >= x.size() && compare(size() - x.size(), npos, x) == 0;
        }

        BLESSED_CXX14_CONSTEXPR bool ends_with(CharT x) const noexcept
        {
            return ends_with(basic_string_view(&x, 1));
        }

        BLESSED_CXX14_CONSTEXPR bool ends_with(const CharT *x) const
        {
            return ends_with(basic_string_view(x));
        }
#endif

        BLESSED_CXX14_CONSTEXPR size_type find(basic_string_view s, size_type pos = 0) const noexcept
        {
            assert(((s.size() == 0) || (s.data() != nullptr)) && "basic_string_view::find(): received nullptr");
            return blessed::algo::detail::str_find<value_type, size_type, traits_type, npos>(data(), size(), s.data(), pos, s.size());
        }

        BLESSED_CXX14_CONSTEXPR size_type find(CharT ch, size_type pos = 0) const noexcept
        {
            return blessed::algo::detail::str_find<value_type, size_type, traits_type, npos>(data(), size(), ch, pos);
        }

        BLESSED_CXX14_CONSTEXPR size_type find(const CharT *s, size_type pos, size_type count) const
        {
            assert(((count == 0) || (s != nullptr)) && "basic_string_view::find(): received nullptr");
            return blessed::algo::detail::str_find<value_type, size_type, traits_type, npos>(data(), size(), s, pos, count);
        }

        BLESSED_CXX14_CONSTEXPR size_type find(const CharT *s, size_type pos = 0) const
        {
            assert((s != nullptr) && "basic_string_view::find(): received nullptr");
            return blessed::algo::detail::str_find<value_type, size_type, traits_type, npos>(data(), size(), s, pos, traits_type::length(s));
        }

        BLESSED_CXX14_CONSTEXPR size_type rfind(basic_string_view s, size_type pos = npos) const noexcept
        {
            assert(((s.size() == 0) || (s.data() != nullptr)) && "basic_string_view::rfind(): received nullptr");
            return blessed::algo::detail::str_rfind<value_type, size_type, traits_type, npos>(data(), size(), s.data(), pos, s.size());
        }

        BLESSED_CXX14_CONSTEXPR size_type rfind(CharT c, size_type pos = npos) const noexcept
        {
            return blessed::algo::detail::str_rfind<value_type, size_type, traits_type, npos>(data(), size(), c, pos);
        }

        BLESSED_CXX14_CONSTEXPR size_type rfind(const CharT *s, size_type pos, size_type count) const
        {
            assert(((count == 0) || (s != nullptr)) && "basic_string_view::rfind(): received nullptr");
            return blessed::algo::detail::str_rfind<value_type, size_type, traits_type, npos>(data(), size(), s, pos, count);
        }

        BLESSED_CXX14_CONSTEXPR size_type rfind(const CharT *s, size_type pos = npos) const
        {
            assert((s != nullptr) && "basic_string_view::rfind(): received nullptr");
            return blessed::algo::detail::str_rfind<value_type, size_type, traits_type, npos>(data(), size(), s, pos, traits_type::length(s));
        }

        BLESSED_CXX14_CONSTEXPR size_type find_first_of(basic_string_view s, size_type pos = 0) const noexcept
        {
            assert(((s.size() == 0) || (s.data() != nullptr)) && "basic_string_view::find_first_of(): received nullptr");
            return blessed::algo::detail::str_find_first_of<value_type, size_type, traits_type, npos>(data(), size(), s.data(), pos, s.size());
        }

        BLESSED_CXX14_CONSTEXPR size_type find_first_of(CharT c, size_type pos = 0) const noexcept
        {
            return find(c, pos);
        }

        BLESSED_CXX14_CONSTEXPR size_type find_first_of(const CharT *s, size_type pos, size_type count) const
        {
            assert(((count == 0) || (s != nullptr)) && "basic_string_view::find_first_of(): received nullptr");
            return blessed::algo::detail::str_find_first_of<value_type, size_type, traits_type, npos>(data(), size(), s, pos, count);
        }

        BLESSED_CXX14_CONSTEXPR size_type find_first_of(const CharT *s, size_type pos = 0) const
        {
            assert((s != nullptr) && "basic_string_view::find_first_of(): received nullptr");
            return blessed::algo::detail::str_find_first_of<value_type, size_type, traits_type, npos>(
                data(), size(), s, pos, traits_type::length(s));
        }

        BLESSED_CXX14_CONSTEXPR size_type find_last_of(basic_string_view s, size_type pos = npos) const noexcept
        {
            assert(((s.size() == 0) || (s.data() != nullptr)) && "basic_string_view::find_last_of(): received nullptr");
            return blessed::algo::detail::str_find_last_of<value_type, size_type, traits_type, npos>(data(), size(), s.data(), pos, s.size());
        }

        BLESSED_CXX14_CONSTEXPR size_type find_last_of(CharT c, size_type pos = npos) const noexcept
        {
            return rfind(c, pos);
        }

        BLESSED_CXX14_CONSTEXPR size_type find_last_of(const CharT *s, size_type pos, size_type count) const
        {
            assert(((count == 0) || (s != nullptr)) && "basic_string_view::find_last_of(): received nullptr");
            return blessed::algo::detail::str_find_last_of<value_type, size_type, traits_type, npos>(data(), size(), s, pos, count);
        }

        BLESSED_CXX14_CONSTEXPR size_type find_last_of(const CharT *s, size_type pos = npos) const
        {
            assert((s != nullptr) && "basic_string_view::find_last_of(): received nullptr");
            return blessed::algo::detail::str_find_last_of<value_type, size_type, traits_type, npos>(
                data(), size(), s, pos, traits_type::length(s));
        }

        BLESSED_CXX14_CONSTEXPR size_type find_first_not_of(basic_string_view s, size_type pos = 0) const noexcept
        {
            assert(((s.size() == 0) || (s.data() != nullptr)) && "basic_string_view::find_first_not_of(): received nullptr");
            return blessed::algo::detail::str_find_first_not_of<value_type, size_type, traits_type, npos>(
                data(), size(), s.data(), pos, s.size());
        }

        BLESSED_CXX14_CONSTEXPR size_type find_first_not_of(CharT c, size_type pos = 0) const noexcept
        {
            return blessed::algo::detail::str_find_first_not_of<value_type, size_type, traits_type, npos>(data(), size(), c, pos);
        }

        BLESSED_CXX14_CONSTEXPR size_type find_first_not_of(const CharT *s, size_type pos, size_type count) const
        {
            assert(((count == 0) || (s != nullptr)) && "basic_string_view::find_first_not_of(): received nullptr");
            return blessed::algo::detail::str_find_first_not_of<value_type, size_type, traits_type, npos>(data(), size(), s, pos, count);
        }

        BLESSED_CXX14_CONSTEXPR size_type find_first_not_of(const CharT *s, size_type pos = 0) const
        {
            assert((s != nullptr) && "basic_string_view::find_first_not_of(): received nullptr");
            return blessed::algo::detail::str_find_first_not_of<value_type, size_type, traits_type, npos>(
                data(), size(), s, pos, traits_type::length(s));
        }

        BLESSED_CXX14_CONSTEXPR size_type find_last_not_of(basic_string_view s, size_type pos = npos) const noexcept
        {
            assert(((s.size() == 0) || (s.data() != nullptr)) && "basic_string_view::find_last_not_of(): received nullptr");
            return blessed::algo::detail::str_find_last_not_of<value_type, size_type, traits_type, npos>(
                data(), size(), s.data(), pos, s.size());
        }

        BLESSED_CXX14_CONSTEXPR size_type find_last_not_of(CharT c, size_type pos = npos) const noexcept
        {
            return blessed::algo::detail::str_find_last_not_of<value_type, size_type, traits_type, npos>(data(), size(), c, pos);
        }

        BLESSED_CXX14_CONSTEXPR size_type find_last_not_of(const CharT *s, size_type pos, size_type count) const
        {
            assert(((count == 0) || (s != nullptr)) && "basic_string_view::find_last_not_of(): received nullptr");
            return blessed::algo::detail::str_find_last_not_of<value_type, size_type, traits_type, npos>(data(), size(), s, pos, count);
        }

        BLESSED_CXX14_CONSTEXPR size_type find_last_not_of(const CharT *s, size_type pos = npos) const
        {
            assert((s != nullptr) && "basic_string_view::find_last_not_of(): received nullptr");
            return blessed::algo::detail::str_find_last_not_of<value_type, size_type, traits_type, npos>(
                data(), size(), s, pos, traits_type::length(s));
        }

        operator std::string() const
        {
            return std::string(data(), size());
        }
    };

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool operator==(basic_string_view<CharT, Traits> lhs, basic_string_view<CharT, Traits> rhs) noexcept
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        return lhs.compare(rhs) == 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool
    operator==(basic_string_view<CharT, Traits> lhs, blessed::common_type_t<basic_string_view<CharT, Traits>> rhs) noexcept
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        return lhs.compare(rhs) == 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool
    operator==(blessed::common_type_t<basic_string_view<CharT, Traits>> lhs, basic_string_view<CharT, Traits> rhs) noexcept
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        return lhs.compare(rhs) == 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool operator!=(basic_string_view<CharT, Traits> lhs, basic_string_view<CharT, Traits> rhs) noexcept
    {
        if (lhs.size() != rhs.size())
        {
            return true;
        }

        return lhs.compare(rhs) != 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool
    operator!=(basic_string_view<CharT, Traits> lhs, blessed::common_type_t<basic_string_view<CharT, Traits>> rhs) noexcept
    {
        if (lhs.size() != rhs.size())
        {
            return true;
        }

        return lhs.compare(rhs) != 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool
    operator!=(blessed::common_type_t<basic_string_view<CharT, Traits>> lhs, basic_string_view<CharT, Traits> rhs) noexcept
    {
        if (lhs.size() != rhs.size())
        {
            return true;
        }

        return lhs.compare(rhs) != 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool operator<(basic_string_view<CharT, Traits> lhs, basic_string_view<CharT, Traits> rhs) noexcept
    {
        return lhs.compare(rhs) < 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool
    operator<(basic_string_view<CharT, Traits> lhs, blessed::common_type_t<basic_string_view<CharT, Traits>> rhs) noexcept
    {
        return lhs.compare(rhs) < 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool
    operator<(blessed::common_type_t<basic_string_view<CharT, Traits>> lhs, basic_string_view<CharT, Traits> rhs) noexcept
    {
        return lhs.compare(rhs) < 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool operator<=(basic_string_view<CharT, Traits> lhs, basic_string_view<CharT, Traits> rhs) noexcept
    {
        return lhs.compare(rhs) <= 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool
    operator<=(basic_string_view<CharT, Traits> lhs, blessed::common_type_t<basic_string_view<CharT, Traits>> rhs) noexcept
    {
        return lhs.compare(rhs) <= 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool
    operator<=(blessed::common_type_t<basic_string_view<CharT, Traits>> lhs, basic_string_view<CharT, Traits> rhs) noexcept
    {
        return lhs.compare(rhs) <= 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool operator>(basic_string_view<CharT, Traits> lhs, basic_string_view<CharT, Traits> rhs) noexcept
    {
        return lhs.compare(rhs) > 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool
    operator>(basic_string_view<CharT, Traits> lhs, blessed::common_type_t<basic_string_view<CharT, Traits>> rhs) noexcept
    {
        return lhs.compare(rhs) > 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool
    operator>(blessed::common_type_t<basic_string_view<CharT, Traits>> lhs, basic_string_view<CharT, Traits> rhs) noexcept
    {
        return lhs.compare(rhs) > 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool operator>=(basic_string_view<CharT, Traits> lhs, basic_string_view<CharT, Traits> rhs) noexcept
    {
        return lhs.compare(rhs) >= 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool
    operator>=(basic_string_view<CharT, Traits> lhs, blessed::common_type_t<basic_string_view<CharT, Traits>> rhs) noexcept
    {
        return lhs.compare(rhs) >= 0;
    }

    template <typename CharT, typename Traits>
    BLESSED_CXX14_CONSTEXPR bool
    operator>=(blessed::common_type_t<basic_string_view<CharT, Traits>> lhs, basic_string_view<CharT, Traits> rhs) noexcept
    {
        return lhs.compare(rhs) >= 0;
    }

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, basic_string_view<CharT, Traits> v)
    {
        typename std::basic_ostream<CharT, Traits>::sentry sentry(os);

        if (sentry)
        {
            size_t lpad = 0;
            size_t rpad = 0;

            if (static_cast<size_t>(os.width()) > v.size())
            {
                size_t pad = os.width() - static_cast<std::streamsize>(v.size());

                if ((os.flags() & os.adjustfield) == os.left)
                {
                    rpad = pad;
                }
                else
                {
                    lpad = pad;
                }
            }

            auto write_padding = [](std::ostream &o, size_t pad)
            {
                char fill_buf[32];
                memset(fill_buf, o.fill(), sizeof(fill_buf));

                while (pad)
                {
                    size_t n = std::min(pad, sizeof(fill_buf));
                    o.write(fill_buf, static_cast<std::streamsize>(n));
                    pad -= n;
                }
            };

            if (lpad)
            {
                write_padding(os, lpad);
            }

            os.write(v.data(), static_cast<std::streamsize>(v.size()));

            if (rpad)
            {
                write_padding(os, static_cast<std::streamsize>(rpad));
            }

            os.width(0);
        }

        return os;
    }

    using string_view = basic_string_view<char>;
    using ci_string_view = basic_string_view<char, blessed::detail::case_insensitive_char_traits<char>>;

    using wstring_view = basic_string_view<wchar_t>;
    using u16string_view = basic_string_view<char16_t>;
    using u32string_view = basic_string_view<char32_t>;

    template <typename CharT, typename Traits>
    BLESSED_CONSTEXPR_VALUE typename basic_string_view<CharT, Traits>::size_type
        basic_string_view<CharT, Traits>::npos = std::numeric_limits<typename basic_string_view<CharT, Traits>::size_type>::max();

    inline namespace literals
    {

        inline namespace string_view_literals
        {

            inline BLESSED_CXX14_CONSTEXPR string_view operator"" _sv(const char *str, std::size_t len) noexcept
            {
                return {str, len};
            }

            inline BLESSED_CXX14_CONSTEXPR wstring_view operator"" _sv(const wchar_t *str, std::size_t len) noexcept
            {
                return {str, len};
            }

            inline BLESSED_CXX14_CONSTEXPR u16string_view operator"" _sv(const char16_t *str, std::size_t len) noexcept
            {
                return {str, len};
            }

            inline BLESSED_CXX14_CONSTEXPR u32string_view operator"" _sv(const char32_t *str, std::size_t len) noexcept
            {
                return {str, len};
            }

            inline BLESSED_CXX14_CONSTEXPR ci_string_view operator"" _ci_sv(const char *str, std::size_t len) noexcept
            {
                return {str, len};
            }
        }
    }
}

#else

#include <blessed/detail/char_traits.hpp>
#include <blessed/hash.hpp>

#include <string_view>

namespace blessed
{
    using ::std::basic_string_view;
    using ::std::string_view;

    using ci_string_view = std::basic_string_view<char, blessed::detail::case_insensitive_char_traits<char>>;

    inline namespace literals
    {

        inline namespace string_view_literals
        {

            inline BLESSED_CXX14_CONSTEXPR blessed::string_view operator"" _sv(const char *str, std::size_t len) noexcept
            {
                return blessed::string_view(str, len);
            }

            inline BLESSED_CXX14_CONSTEXPR ci_string_view operator"" _ci_sv(const char *str, std::size_t len) noexcept
            {
                return ci_string_view(str, len);
            }

        }
    }
}

#endif

namespace blessed
{
    template <typename CharT, typename Traits>
    struct hash<blessed::basic_string_view<CharT, Traits>>
    {
        size_t operator()(const blessed::basic_string_view<CharT, Traits> s) const noexcept
        {
            using value_type = blessed::remove_pointer_t<decltype(s.data())>;
            return blessed::hash_detail::CityHash{}(s.data(), s.size() * sizeof(value_type));
        }
    };
}

namespace std
{
    template <typename CharT, typename Traits>
    struct hash<blessed::basic_string_view<CharT, Traits>>
    {
        size_t operator()(const blessed::basic_string_view<CharT, Traits> s) const noexcept
        {
            using value_type = blessed::remove_pointer_t<decltype(s.data())>;
            return blessed::hash_detail::CityHash{}(s.data(), s.size() * sizeof(value_type));
        }
    };
}

#endif
