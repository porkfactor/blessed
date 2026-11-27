#ifndef BLESSED_STATIC_STRING_HPP
#define BLESSED_STATIC_STRING_HPP

#include <blessed/blessed.hpp>

#include <blessed/detail/string.hpp>
#include <blessed/iterator.hpp>
#include <blessed/string_view.hpp>
#include <blessed/detail/char_traits.hpp>
#include <blessed/detail/static_vector_impl.hpp>
#include <blessed/align.hpp>

#include <cassert>
#include <cstddef>
#include <memory>
#include <string>

namespace blessed
{
    namespace detail
    {
        template <typename CharT, typename Traits, typename T>
        struct can_be_converted_to_string_view :
            public std::integral_constant<
                bool,
                std::is_convertible<const T &, blessed::basic_string_view<CharT, Traits>>::value &&
                    !std::is_convertible<const T &, const CharT *>::value>
        {};

        template <typename Alloc, typename = void, typename = void>
        struct is_allocator : std::false_type
        {};

        template <typename Alloc>
        struct is_allocator<
            Alloc,
            blessed::void_t<typename Alloc::value_type>,
            blessed::void_t<decltype(std::declval<Alloc &>().allocate(size_t(0)))>> : std::true_type
        {};
    }

    template <typename CharT, typename Traits, typename Storage, typename Allocator = std::allocator<CharT>>
    class static_string_impl : public static_vector_impl<CharT, Storage, Allocator>
    {
        using super = static_vector_impl<CharT, Storage, Allocator>;

    protected:
        explicit static_string_impl(std::size_t n, const Allocator &a) : super(n, a) {}

    public:
        using allocator_type = typename super::allocator_type;
        using traits_type = Traits;

        using string_view_type = blessed::basic_string_view<CharT, Traits>;

        using value_type = typename super::value_type;
        using size_type = typename super::size_type;
        using iterator = typename super::iterator;
        using const_iterator = typename super::const_iterator;
        using reference = typename super::reference;
        using const_reference = typename super::const_reference;
        using pointer = typename super::pointer;
        using const_pointer = typename super::const_pointer;

        static const size_type npos;

        static_string_impl(const static_string_impl &) = delete;

        static_string_impl &operator=(const static_string_impl &rhs)
        {
            if (this == &rhs)
            {
                return *this;
            }

            // If we have sufficient space, assign the common elements, then destroy any excess
            size_type rhs_size = rhs.size();
            size_type cur_size = this->size();
            if (cur_size >= rhs_size)
            {
                // assign the common elements
                iterator new_end = this->begin();

                if (rhs_size)
                {
                    new_end = std::copy(rhs.begin(), rhs.begin() + rhs_size, this->begin());
                }

                // trim
                this->set_size(static_cast<size_type>(new_end - this->begin()));
                traits_type::assign(this->data()[this->size()], '\0');
                return *this;
            }

            if (this->capacity() < rhs.size())
            {
                assert(0 && "Cannot assign static_string_impl not enough space");
            }
            else if (cur_size)
            {
                // Otherwise, use assignment for the already-constructed elements.
                std::copy(rhs.begin(), rhs.begin() + cur_size, this->begin());
            }

            // Copy construct the new elements in place.
            std::uninitialized_copy(rhs.begin() + cur_size, rhs.end(), this->begin() + cur_size);
            this->set_size(rhs_size);
            traits_type::assign(this->data()[this->size()], '\0');

            return *this;
        }

        static_string_impl &operator=(static_string_impl &&rhs) noexcept
        {
            if (this == &rhs)
            {
                return *this;
            }

            // If we have sufficient space, assign the common elements, then destroy any excess
            size_type rhs_size = rhs.size();
            size_type cur_size = this->size();
            if (cur_size >= rhs_size)
            {
                // assign the common elements
                iterator new_end = this->begin();

                if (rhs_size)
                {
                    new_end = std::move(rhs.begin(), rhs.begin() + rhs_size, this->begin());
                }

                // trim
                this->set_size(static_cast<size_type>(new_end - this->begin()));
                traits_type::assign(this->data()[this->size()], '\0');

                rhs.clear();

                return *this;
            }

            if (this->capacity() < rhs.size())
            {
                assert(0 && "Cannot assign static_string_impl not enough space");
            }
            else if (cur_size)
            {
                // Otherwise, use assignment for the already-constructed elements.
                std::move(rhs.begin(), rhs.begin() + cur_size, this->begin());
            }

            // Copy construct the new elements in place.
            std::uninitialized_copy(
                std::make_move_iterator(rhs.begin() + cur_size), std::make_move_iterator(rhs.end()), this->begin() + cur_size);
            this->set_size(rhs_size);
            traits_type::assign(this->data()[this->size()], '\0');

            rhs.clear();

            return *this;
        }

        ~static_string_impl() = default;

        allocator_type get_allocator() const
        {
            return this->get_allocator_ref();
        }

        std::size_t length() const
        {
            return this->size();
        }

        void append(const_iterator from, size_type count, size_type pos)
        {
            size_type const num = std::min(count, this->capacity() - 1) - pos;
            traits_type::copy(this->end(), from + pos, num);
            this->set_size(this->size() + num);
            traits_type::assign(this->data()[this->size()], '\0');
        }

        void replace(const_iterator first, size_type count, const_iterator first2, size_type count2)
        {
            if (count != count2)
            {
                size_type const adjusted_size = std::min((this->size() + count2) - count, this->capacity() - 1);
                size_type const pos = static_cast<size_type>(std::distance(this->cbegin(), first));
                size_type const copy_size = ((count2 + pos) > (this->capacity() - 1)) ? (this->capacity() - (1 + pos)) : count2;

                traits_type::move(const_cast<iterator>(first) + count2, first + count, static_cast<size_type>(std::distance(first + count, this->cend())));
                traits_type::copy(const_cast<iterator>(first), first2, copy_size);

                this->set_size(adjusted_size);

                traits_type::assign(this->data()[this->size()], '\0');
            }
            else
            {
                traits_type::copy(const_cast<iterator>(first), first2, count2);
            }
        }

        void replace(const_iterator first, size_type count, CharT ch, size_type count2)
        {
            if (count != count2)
            {
                size_type const adjusted_size = std::min((this->size() + count2) - count, this->capacity() - 1);
                size_type const pos = static_cast<size_type>(std::distance(this->cbegin(), first));
                size_type const fill_size = ((pos + count2) > (this->capacity() - 1)) ? (this->capacity() - (1 + pos)) : count2;

                traits_type::assign(const_cast<iterator>(first), fill_size, ch);

                this->set_size(adjusted_size);

                traits_type::assign(this->data()[this->size()], '\0');
            }
            else
            {
                traits_type::assign(const_cast<iterator>(first), count, ch);
            }
        }

        size_type copy(pointer dest, const_iterator from BLESSED_UNUSED, size_type count, size_type pos) const
        {
            if (count > 0)
            {
                size_type const num = std::min(count - 1, this->capacity() - 1) - pos;
                traits_type::copy(dest, this->data() + pos, num);
                traits_type::assign(*(dest + num), '\0');
                return num;
            }

            return 0;
        }

        static_string_impl &assign(size_type count, CharT ch)
        {
            this->set_size(0);
            append(count, ch);
            return *this;
        }

        static_string_impl &assign(const static_string_impl &str)
        {
            return *this = str;
        }

        static_string_impl &assign(const static_string_impl &str, size_type pos, size_type count = npos)
        {
            return *this = string_view_type(str.data() + pos, count);
        }

        static_string_impl &assign(static_string_impl &&str)
        {
            return *this = std::move(str);
        }

        static_string_impl &assign(const CharT *s, size_type count)
        {
            return assign(string_view_type(s, count));
        }

        static_string_impl &assign(const CharT *s)
        {
            return assign(string_view_type(s));
        }

        template <typename InputIt>
        static_string_impl &assign(InputIt first, InputIt last)
        {
            clear();

            for (; first != last; ++first)
            {
                push_back(*first);
            }

            return *this;
        }

        static_string_impl &assign(std::initializer_list<CharT> ilist)
        {
            assign(string_view_type(ilist.begin(), ilist.size()));
            return *this;
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        static_string_impl &assign(const T &t)
        {
            string_view_type sv = t;
            this->set_size(0);
            append(sv.data(), sv.length(), 0);
            return *this;
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        static_string_impl &assign(const T &t, size_type pos, size_type count = npos)
        {
            string_view_type sv = t;
            this->set_size(0);
            append(sv.data(), pos + count, pos);
            return *this;
        }

        reference operator[](size_type pos)
        {
            assert(pos < this->size() && "static_vector::operator[]() out of range");

            return this->data()[pos];
        }

        const_reference operator[](size_type pos) const
        {
            assert(pos < this->size() && "static_vector::operator[]() out of range");

            return this->data()[pos];
        }

        reference at(size_type pos)
        {
            // TODO exception path
            return operator[](pos);
        }

        const_reference at(size_type pos) const
        {
            // TODO exception path
            return operator[](pos);
        }

        const CharT *c_str() const
        {
            return this->data();
        }

        operator string_view_type() const noexcept
        {
            return string_view_type(this->data(), this->size());
        }

        static_string_impl &insert(size_type index, size_type count, CharT ch)
        {
            assert(index <= this->size() && "static_string_impl::insert() index out of range");

            size_type const num = std::min(count, this->capacity() - index - count);
            iterator const dest = this->begin() + index + count;
            iterator const source = this->begin() + index;
            auto const move_n = static_cast<size_type>(this->end() - source);

            traits_type::move(dest, source, move_n);
            traits_type::assign(source, num, ch);

            this->set_size(this->size() + num);
            traits_type::assign(this->data()[this->size()], '\0');

            return *this;
        }

        static_string_impl &insert(size_type index, const CharT *s)
        {
            assert(index <= this->size() && "static_string_impl::insert() index out of range");

            insert(index, s, traits_type::length(s));

            return *this;
        }

        static_string_impl &insert(size_type index, const CharT *s, size_type count)
        {
            assert(index <= this->size() && "static_string_impl::insert() index out of range");

            size_type const num = std::min(count, this->capacity() - index - count);
            iterator const dest = this->begin() + index + count;
            iterator const source = this->begin() + index;
            auto const move_n = static_cast<size_type>(this->end() - source);

            traits_type::move(dest, source, move_n);
            traits_type::copy(source, s, num);

            this->set_size(this->size() + num);
            traits_type::assign(this->data()[this->size()], '\0');

            return *this;
        }

        static_string_impl &insert(size_type index, const static_string_impl &str)
        {
            assert(index <= this->size() && "static_string_impl::insert() index out of range");

            insert(index, str.data(), str.length());

            return *this;
        }

        static_string_impl &insert(size_type index, const static_string_impl &str, size_type index_str, size_type count = npos)
        {
            size_t num = count == npos ? str.size() : count;

            assert(index <= this->size() && "static_string_impl::insert() index out of range");
            assert(index_str <= str.size() && "static_string_impl::insert() index_str out of range");

            return insert(index, str.substr(index_str, num));
        }

        iterator insert(const_iterator pos, CharT ch)
        {
            insert(static_cast<size_type>(pos - this->begin()), 1, ch);

            return iterator(std::addressof(*pos));
        }

        iterator insert(const_iterator pos, size_type count, CharT ch)
        {
            insert(static_cast<size_type>(pos - this->begin()), count, ch);

            return iterator(std::addressof(*pos));
        }

        template <typename InputIt>
        iterator insert(const_iterator pos, InputIt first, InputIt last)
        {
            assert(pos >= this->begin() && pos <= this->end());

            for (; first != last; ++first)
            {
                if (this->size() + 1 >= this->max_size())
                    break;

                this->insert(pos++, *first);
            }

            return iterator(std::addressof(*pos));
        }

        iterator insert(const_iterator pos, std::initializer_list<CharT> il)
        {
            insert(static_cast<size_type>(pos - this->begin()), il.begin(), il.size());

            return iterator(std::addressof(*pos));
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        static_string_impl &insert(size_type index, const T &t)
        {
            assert(index <= this->size() && "static_string_impl::insert() index out of range");

            string_view_type sv = t;

            insert(index, sv.data(), sv.length());

            return *this;
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        static_string_impl &insert(size_type index, const T &t, size_type index_str, size_type count = npos)
        {
            assert(index <= this->size() && "static_string_impl::insert() index out of range");

            string_view_type sv = t;

            size_t num = count == npos ? sv.size() - index_str : count;

            assert(index_str <= sv.size() && "static_string_impl::insert() index_str out of range");

            insert(index, sv.data() + index_str, num);

            return *this;
        }

        static_string_impl &erase(size_type index = 0, size_type count = npos)
        {
            assert(index <= this->size() && "static_string_impl::erase() index out of range");

            if (count != npos)
            {
                assert(count <= this->size() && "static_string_impl::erase() count out of range");

                iterator const dest = this->begin() + index;
                iterator const source = this->begin() + index + count;
                auto const move_n = static_cast<size_type>(this->end() - source);

                traits_type::move(dest, source, move_n);

                this->set_size(this->size() - std::min(count, this->size() - index));
            }
            else
            {
                this->set_size(index);
            }

            traits_type::assign(this->data()[this->size()], '\0');

            return *this;
        }

        iterator erase(const_iterator pos)
        {
            return erase(pos, pos + 1);
        }

        iterator erase(const_iterator first, const_iterator last)
        {
            auto const move_n = static_cast<size_type>(std::distance(last, this->cend()));
            auto const count = static_cast<size_type>(std::distance(first, last));

            traits_type::move(iterator(std::addressof(*first)), last, move_n);
            this->set_size(this->size() - count);
            traits_type::assign(this->data()[this->size()], '\0');

            return iterator(std::addressof(*first));
        }

        void push_back(CharT ch)
        {
            append(1, ch);
        }

        void pop_back()
        {
            this->set_size(this->size() - 1);
            traits_type::assign(this->data()[this->size()], '\0');
        }

        static_string_impl &append(size_type count, CharT ch)
        {
            size_type const num = std::min(this->capacity() - this->size() - 1, count);
            traits_type::assign(this->end(), num, ch);
            this->set_size(this->size() + num);
            traits_type::assign(this->data()[this->size()], '\0');
            return *this;
        }

        static_string_impl &append(const static_string_impl &str)
        {
            append(str.cbegin(), str.length(), 0);
            return *this;
        }

        static_string_impl &append(const static_string_impl &str, size_type pos, size_type count = npos)
        {
            append(str.cbegin(), pos + count, pos);
            return *this;
        }

        static_string_impl &append(const CharT *s, size_type count)
        {
            append(const_iterator(s), count, 0);
            return *this;
        }

        static_string_impl &append(const CharT *s)
        {
            append(const_iterator(s), traits_type::length(s), 0);
            return *this;
        }

        template <typename InputIt>
        static_string_impl &append(InputIt first, InputIt last)
        {
            for (; first != last; ++first)
            {
                push_back(*first);
            }

            return *this;
        }

        static_string_impl &append(std::initializer_list<CharT> il)
        {
            append(il.begin(), il.size(), 0);
            return *this;
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        static_string_impl &append(const T &t)
        {
            string_view_type sv = t;
            append(sv.data(), sv.length(), 0);
            return *this;
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        static_string_impl &append(const T &t, size_type pos, size_type count = npos)
        {
            string_view_type sv = t;
            append(sv.data(), pos + count, pos);
            return *this;
        }

        static_string_impl &operator+=(const static_string_impl &str)
        {
            return append(str);
        }

        static_string_impl &operator+=(CharT ch)
        {
            return append(1, ch);
        }

        static_string_impl &operator+=(const CharT *s)
        {
            return append(s);
        }

        static_string_impl &operator+=(std::initializer_list<CharT> ilist)
        {
            return append(ilist);
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        static_string_impl &operator+=(const T &t)
        {
            return append(t);
        }

        int compare(const static_string_impl &str) const
        {
            return string_view_type(*this).compare(str);
        }

        int compare(size_type pos1, size_type count1, const static_string_impl &str) const
        {
            return string_view_type(*this).compare(pos1, count1, str);
        }

        int compare(const CharT *s) const
        {
            return string_view_type(*this).compare(s);
        }

        int compare(size_type pos1, size_type count1, const CharT *s, size_type count2) const
        {
            return string_view_type(*this).compare(pos1, count1, s, count2);
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        int compare(const T &t) const
        {
            string_view_type sv = t;
            return string_view_type(*this).compare(sv);
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        int compare(size_type pos1, size_type count1, const T &t) const
        {
            string_view_type sv = t;
            return string_view_type(*this).compare(pos1, count1, sv);
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        int compare(size_type pos1, size_type count1, const T &t, size_type pos2, size_type count2 = npos) const
        {
            string_view_type sv = t;
            return string_view_type(*this).compare(pos1, count1, sv, pos2, count2);
        }

#if (defined(__cpp_lib_starts_ends_with) && __cpp_lib_starts_ends_with >= 201711)
        bool starts_with(string_view_type x) const noexcept
        {
            return string_view_type(this->data(), this->size()).starts_with(x);
        }

        bool starts_with(CharT x) const noexcept
        {
            return string_view_type(this->data(), this->size()).starts_with(x);
        }

        bool starts_with(const CharT *x) const
        {
            return string_view_type(this->data(), this->size()).starts_with(x);
        }

        bool ends_with(string_view_type x) const noexcept
        {
            return string_view_type(this->data(), this->size()).ends_with(x);
        }

        bool ends_with(CharT x) const noexcept
        {
            return string_view_type(this->data(), this->size()).ends_with(x);
        }

        bool ends_with(const CharT *x) const
        {
            return string_view_type(this->data(), this->size()).ends_with(x);
        }
#endif

        static_string_impl &replace(size_type pos, size_type count, const static_string_impl &str)
        {
            assert(pos <= this->size() && "static_string_impl::replace() pos out of range");
            assert(((pos + count) <= this->max_size()) && "static_string_impl::replace() out of space");

            replace(this->begin() + pos, count, str.begin(), str.length());

            return *this;
        }

        static_string_impl &replace(const_iterator first, const_iterator last, const static_string_impl &str)
        {
            return replace(static_cast<size_type>(first - this->begin()), static_cast<size_type>(std::distance(first, last)), str);
        }

        static_string_impl &replace(size_type pos, size_type count, const static_string_impl &str, size_type pos2, size_type count2 = npos)
        {
            assert(pos <= this->size() && "static_string_impl::replace() pos out of range");
            assert(pos2 <= str.length() && "static_string_impl::replace() pos2 out of range");
            assert(((pos + count) <= this->max_size()) && "static_string_impl::replace() out of space");

            size_type const size2 = std::min(count2, str.size());
            replace(this->begin() + pos, count, str.begin() + pos2, size2);

            return *this;
        }

        template <typename InputIt>
        static_string_impl &replace(const_iterator first, const_iterator last, InputIt first2, InputIt last2)
        {
            iterator mutable_first = const_cast<iterator>(first);
            iterator mutable_last = const_cast<iterator>(last);

            for (; first2 != last2; ++first2)
            {
                if (mutable_first == mutable_last)
                {
                    this->insert(mutable_first, first2, last2);
                    break;
                }

                traits_type::assign(*mutable_first++, *first2);
            }

            if (mutable_first != mutable_last)
            {
                this->erase(mutable_first, mutable_last);
            }

            return *this;
        }

        static_string_impl &replace(size_type pos, size_type count, const CharT *cstr, size_type count2)
        {
            assert(pos <= this->size() && "static_string_impl::replace() pos out of range");
            assert(((pos + count) <= this->max_size()) && "static_string_impl::replace() out of space");

            replace(this->begin() + pos, std::min(count, this->size() - pos), cstr, count2);

            return *this;
        }

        static_string_impl &replace(const_iterator first, const_iterator last, const CharT *cstr, size_type count2)
        {
            return replace(
                static_cast<size_type>(first - this->begin()), static_cast<size_type>(std::distance(first, last)), cstr, count2);
        }

        static_string_impl &replace(size_type pos, size_type count, const CharT *cstr)
        {
            assert(pos <= this->size() && "static_string_impl::replace() pos out of range");
            assert(((pos + count) <= this->max_size()) && "static_string_impl::replace() out of space");

            replace(this->begin() + pos, std::min(count, this->size() - pos), cstr, traits_type::length(cstr));

            return *this;
        }

        static_string_impl &replace(const_iterator first, const_iterator last, const CharT *cstr)
        {
            return replace(static_cast<size_type>(first - this->begin()), static_cast<size_type>(std::distance(first, last)), cstr);
        }

        static_string_impl &replace(size_type pos, size_type count, size_type count2, CharT ch)
        {
            assert(pos <= this->size() && "static_string_impl::replace() pos out of range");
            assert(((pos + count) <= this->max_size()) && "static_string_impl::replace() out of space");

            replace(this->begin() + pos, std::min(count, this->size() - pos), ch, count2);

            return *this;
        }

        static_string_impl &replace(const_iterator first, const_iterator last, size_type count2, CharT ch)
        {
            return replace(
                static_cast<size_type>(first - this->begin()), static_cast<size_type>(std::distance(first, last)), count2, ch);
        }

        static_string_impl &replace(const_iterator first, const_iterator last, std::initializer_list<CharT> il)
        {
            assert(
                static_cast<size_type>(first - this->begin()) <= this->size() && "static_string_impl::replace() first out of range");
            assert(
                (static_cast<size_type>(first - this->begin() + std::distance(first, last)) <= this->max_size()) &&
                "static_string_impl::replace() out of space");

            replace(first, static_cast<size_type>(std::distance(first, last)), il.begin(), il.size());

            return *this;
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        static_string_impl &replace(size_type pos, size_type count, const T &t)
        {
            assert(pos <= this->size() && "static_string_impl::replace() pos out of range");
            assert(((pos + count) <= this->max_size()) && "static_string_impl::replace() out of space");

            string_view_type sv = t;
            replace(this->data() + pos, std::min(count, this->size() - pos), sv.data(), sv.size());

            return *this;
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        static_string_impl &replace(const_iterator first, const_iterator last, const T &t)
        {
            return replace(static_cast<size_type>(first - this->begin()), static_cast<size_type>(std::distance(first, last)), t);
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        static_string_impl &replace(size_type pos, size_type count, const T &t, size_type pos2, size_type count2 = npos)
        {
            assert(pos <= this->size() && "static_string_impl::replace() pos out of range");
            assert(((pos + count) <= this->max_size()) && "static_string_impl::replace() out of space");

            string_view_type sv = t;

            assert(pos2 <= sv.length() && "static_string_impl::replace() pos2 out of range");

            size_type const size2 = std::min(count2, sv.size());
            replace(this->data() + pos, std::min(count, this->size() - pos), sv.data() + pos2, size2);

            return *this;
        }

        size_type copy(CharT *dest, size_type count, size_type pos = 0) const
        {
            assert(pos <= this->size() && "static_string_impl::copy() out of range");

            return copy(dest, this->data() + pos, count, pos);
        }

        void resize(size_type count)
        {
            assert(count <= this->max_size() && "static_string_impl::resize() out of range");

            if (count > this->size())
            {
                append(count - this->size(), CharT());
            }
            else if (count < this->size())
            {
                this->set_size(count);
                traits_type::assign(this->data()[this->size()], '\0');
            }
        }

        void resize(size_type count, CharT ch)
        {
            assert(count <= this->max_size() && "static_string_impl::resize() out of range");

            if (count > this->size())
            {
                append(count - this->size(), ch);
            }
            else if (count < this->size())
            {
                this->set_size(count);
                traits_type::assign(this->data()[this->size()], '\0');
            }
        }

        /**
         * Resize to n, and overwrite data via `size_type op(char *buffer, size_type n)` function
         *
         * This is an implementation of proposed std::string change
         */
        template <typename Operation>
        void resize_and_overwrite(size_type n, Operation op)
        {
            // NOTE: in strings with changable capacity this may need to grow the buffer
            assert(n <= this->max_size() && "static_string_impl::resize_and_overwrite() out of range");

            auto m = static_cast<size_type>(op(this->data(), n));

            assert(m <= n && "op() must not write more than n characters");

            erase(this->begin() + m, this->end());
        }

        size_type find(const static_string_impl &str, size_type pos = 0) const
        {
            return algo::detail::str_find<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), str.data(), pos, str.size());
        }

        size_type find(const CharT *s, size_type pos, size_type count) const
        {
            return algo::detail::str_find<value_type, size_type, traits_type, npos>(this->data(), this->size(), s, pos, count);
        }

        size_type find(const CharT *s, size_type pos = 0) const
        {
            return algo::detail::str_find<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), s, pos, traits_type::length(s));
        }

        size_type find(CharT ch, size_type pos = 0) const
        {
            return algo::detail::str_find<value_type, size_type, traits_type, npos>(this->data(), this->size(), ch, pos);
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        size_type find(const T &t, size_type pos = 0) const
        {
            string_view_type sv = t;
            return algo::detail::str_find<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), sv.data(), pos, sv.size());
        }

        size_type rfind(const static_string_impl &str, size_type pos = npos) const
        {
            return algo::detail::str_rfind<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), str.data(), pos, str.size());
        }

        size_type rfind(const CharT *s, size_type pos, size_type count) const
        {
            return algo::detail::str_rfind<value_type, size_type, traits_type, npos>(this->data(), this->size(), s, pos, count);
        }

        size_type rfind(const CharT *s, size_type pos = npos) const
        {
            return algo::detail::str_rfind<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), s, pos, traits_type::length(s));
        }

        size_type rfind(CharT ch, size_type pos = npos) const
        {
            return algo::detail::str_rfind<value_type, size_type, traits_type, npos>(this->data(), this->size(), ch, pos);
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        size_type rfind(const T &t, size_type pos = npos) const
        {
            string_view_type sv = t;
            return algo::detail::str_rfind<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), sv.data(), pos, sv.size());
        }

        size_type find_first_of(const static_string_impl &str, size_type pos = 0) const
        {
            return algo::detail::str_find_first_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), str.data(), pos, str.size());
        }

        size_type find_first_of(const CharT *s, size_type pos, size_type count) const
        {
            return algo::detail::str_find_first_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), s, pos, count);
        }

        size_type find_first_of(const CharT *s, size_type pos = 0) const
        {
            return algo::detail::str_find_first_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), s, pos, traits_type::length(s));
        }

        size_type find_first_of(CharT ch, size_type pos = 0) const
        {
            return find(ch, pos);
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        size_type find_first_of(const T &t, size_type pos = 0) const
        {
            string_view_type sv = t;
            return algo::detail::str_find_first_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), sv.data(), pos, sv.size());
        }

        size_type find_first_not_of(const static_string_impl &str, size_type pos = 0) const
        {
            return algo::detail::str_find_first_not_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), str.data(), pos, str.size());
        }

        size_type find_first_not_of(const CharT *s, size_type pos, size_type count) const
        {
            return algo::detail::str_find_first_not_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), s, pos, count);
        }

        size_type find_first_not_of(const CharT *s, size_type pos = 0) const
        {
            return algo::detail::str_find_first_not_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), s, pos, traits_type::length(s));
        }

        size_type find_first_not_of(CharT ch, size_type pos = 0) const
        {
            return algo::detail::str_find_first_not_of<value_type, size_type, traits_type, npos>(this->data(), this->size(), ch, pos);
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        size_type find_first_not_of(const T &t, size_type pos = 0) const
        {
            string_view_type sv = t;
            return algo::detail::str_find_first_not_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), sv.data(), pos, sv.size());
        }

        size_type find_last_of(const static_string_impl &str, size_type pos = npos) const
        {
            return algo::detail::str_find_last_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), str.data(), pos, str.size());
        }

        size_type find_last_of(const CharT *s, size_type pos, size_type count) const
        {
            return algo::detail::str_find_last_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), s, pos, count);
        }

        size_type find_last_of(const CharT *s, size_type pos = npos) const
        {
            return algo::detail::str_find_last_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), s, pos, traits_type::length(s));
        }

        size_type find_last_of(CharT ch, size_type pos = npos) const
        {
            return rfind(ch, pos);
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        size_type find_last_of(const T &t, size_type pos = npos) const
        {
            string_view_type sv = t;
            return algo::detail::str_find_last_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), sv.data(), pos, sv.size());
        }

        size_type find_last_not_of(const static_string_impl &str, size_type pos = npos) const
        {
            return algo::detail::str_find_last_not_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), str.data(), pos, str.size());
        }

        size_type find_last_not_of(const CharT *s, size_type pos, size_type count) const
        {
            return algo::detail::str_find_last_not_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), s, pos, count);
        }

        size_type find_last_not_of(const CharT *s, size_type pos = npos) const
        {
            return algo::detail::str_find_last_not_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), s, pos, traits_type::length(s));
        }

        size_type find_last_not_of(CharT ch, size_type pos = npos) const
        {
            return algo::detail::str_find_last_not_of<value_type, size_type, traits_type, npos>(this->data(), this->size(), ch, pos);
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        size_type find_last_not_of(const T &t, size_type pos = npos) const
        {
            string_view_type sv = t;
            return algo::detail::str_find_last_not_of<value_type, size_type, traits_type, npos>(
                this->data(), this->size(), sv.data(), pos, sv.size());
        }

        explicit operator std::string() const
        {
            return std::string(this->data(), this->size());
        }

        void clear()
        {
            this->set_size(0);

            traits_type::assign(this->data()[this->size()], '\0');
        }

        void swap(static_string_impl &rhs)
        {
            if (this == &rhs)
            {
                return;
            }

            assert(rhs.size() <= this->capacity() && "static_string_impl::swap() rhs does not fit");
            assert(this->size() <= rhs.capacity() && "static_string_impl::swap() lhs does not fit");

            // swap the shared elements
            size_type n_shared = std::min(this->size(), rhs.size());

            for (size_type i = 0; i != n_shared; ++i)
            {
                std::swap((*this)[i], rhs[i]);
            }

            // TODO this be move?
            // copy over the extra
            if (this->size() > rhs.size())
            {
                size_type n_diff = this->size() - rhs.size();
                std::uninitialized_copy(this->begin() + n_shared, this->end(), rhs.end());
                rhs.set_size(rhs.size() + n_diff);
                traits_type::assign(rhs.data()[rhs.size()], '\0');
                this->set_size(n_shared);
                traits_type::assign(this->data()[this->size()], '\0');
            }
            else if (rhs.size() > this->size())
            {
                size_type n_diff = rhs.size() - this->size();
                std::uninitialized_copy(rhs.begin() + n_shared, rhs.end(), this->end());
                this->set_size(this->size() + n_diff);
                traits_type::assign(this->data()[this->size()], '\0');
                rhs.set_size(n_shared);
                traits_type::assign(rhs.data()[rhs.size()], '\0');
            }
        }
    };

    template <
        std::size_t N,
        typename CharT = char,
        typename Traits = std::char_traits<CharT>,
        typename Allocator = std::allocator<CharT>,
        typename Storage = blessed::detail::static_vector_storage<CharT, N>>
    class static_string : public static_string_impl<CharT, Traits, Storage, Allocator>
    {
        // using storage = detail::static_string_storage<N, CharT>;

    public:
        using impl_type = static_string_impl<CharT, Traits, Storage, Allocator>;

        using string_view_type = typename impl_type::string_view_type;

        template <std::size_t U>
        struct rebind
        {
            using other = static_string<U, CharT, Traits, Allocator>;
        };

        static_string() noexcept(noexcept(Allocator())) : impl_type(N, Allocator())
        {
            this->clear();
        }

        template <typename = blessed::enable_if_t<detail::is_allocator<Allocator>::value, std::nullptr_t>>
        static_string(const CharT *s) : static_string()
        {
            this->append(s);
        }

        explicit static_string(const Allocator &a) : impl_type(N, a)
        {
            this->clear();
        }

        static_string(std::size_t count, CharT ch) : impl_type(N, Allocator())
        {
            this->append(count, ch);
        }

        template <typename = blessed::enable_if_t<detail::is_allocator<Allocator>::value, std::nullptr_t>>
        static_string(std::size_t count, CharT ch, const Allocator &a) : impl_type(N, a)
        {
            this->append(count, ch);
        }

        static_string(const static_string &rhs, std::size_t pos, const Allocator &a = Allocator()) : impl_type(N, a)
        {
            this->append(rhs.cbegin(), rhs.size(), pos);
        }

        static_string(const static_string &rhs, std::size_t pos, std::size_t count, const Allocator &a = Allocator())
            : impl_type(N, a)
        {
            if (count == impl_type::npos)
            {
                this->append(rhs.cbegin(), rhs.size(), pos);
            }
            else
            {
                this->append(rhs.cbegin(), pos + count, pos);
            }
        }

        static_string(const CharT *s, std::size_t count, const Allocator &a = Allocator()) : impl_type(N, a)
        {
            this->append(s, count, 0);
        }

        template <typename = blessed::enable_if_t<detail::is_allocator<Allocator>::value, std::nullptr_t>>
        static_string(const CharT *s, const Allocator &a) : impl_type(N, a)
        {
            this->append(s, Traits::length(s), 0);
        }

        template <typename InputIt>
        static_string(InputIt first, InputIt last, const Allocator &a = Allocator()) : impl_type(N, a)
        {
            this->append(first, last);
        }

        static_string(const impl_type &rhs) : impl_type(N, Allocator())
        {
            if (!rhs.empty())
            {
                impl_type::operator=(rhs);
            }
        }

        static_string(const static_string &rhs) : impl_type(N, Allocator())
        {
            if (!rhs.empty())
            {
                impl_type::operator=(rhs);
            }
        }

        static_string(const impl_type &rhs, const Allocator &a) : impl_type(N, a)
        {
            if (!rhs.empty())
            {
                impl_type::operator=(rhs);
            }
        }

        static_string(const static_string &rhs, const Allocator &a) : impl_type(N, a)
        {
            if (!rhs.empty())
            {
                impl_type::operator=(rhs);
            }
        }

        static_string(impl_type &&rhs) noexcept : impl_type(N, Allocator())
        {
            if (!rhs.empty())
            {
                impl_type::operator=(std::move(rhs));
            }
        }

        static_string(static_string &&rhs) noexcept : impl_type(N, Allocator())
        {
            if (!rhs.empty())
            {
                impl_type::operator=(std::move(rhs));
            }
        }

        static_string(static_string &&rhs, const Allocator &a) : impl_type(N, a)
        {
            if (!rhs.empty())
            {
                impl_type::operator=(std::move(rhs));
            }
        }

        static_string(impl_type &&rhs, const Allocator &a) : impl_type(N, a)
        {
            if (!rhs.empty())
            {
                impl_type::operator=(std::move(rhs));
            }
        }

        static_string(std::initializer_list<CharT> il, const Allocator &a = Allocator()) : impl_type(N, a)
        {
            this->append(il.begin(), il.size(), 0);
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        explicit static_string(const T &t) : impl_type(N, Allocator())
        {
            string_view_type sv = t;
            this->append(sv.data(), sv.length(), 0);
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        explicit static_string(const T &t, const Allocator &a) : impl_type(N, a)
        {
            string_view_type sv = t;
            this->append(sv.data(), sv.length(), 0);
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        static_string(const T &t, std::size_t pos, std::size_t n, const Allocator &a = Allocator()) : impl_type(N, a)
        {
            string_view_type sv = t;
            this->append(sv.data(), pos + n, pos);
        }

        static_string &operator=(const static_string &rhs)
        {
            impl_type::operator=(rhs);
            return *this;
        }

        static_string &operator=(static_string &&rhs) noexcept
        {
            impl_type::operator=(std::move(rhs));
            return *this;
        }

        static_string &operator=(const CharT *s)
        {
            this->assign(s);
            return *this;
        }

        static_string &operator=(CharT ch)
        {
            this->assign(1, ch);
            return *this;
        }

        static_string &operator=(std::initializer_list<CharT> il)
        {
            this->assign(il);
            return *this;
        }

        template <typename T, typename = blessed::enable_if_t<detail::can_be_converted_to_string_view<CharT, Traits, T>::value, void>>
        static_string &operator=(const T &t)
        {
            string_view_type sv = t;
            this->assign(sv);
            return *this;
        }

        static_string substr(std::size_t pos = 0, std::size_t count = impl_type::npos) const
        {
            assert(pos <= this->size() && "static_string::substr() out of range");

            if ((count != impl_type::npos) && ((pos + count) < this->size()))
            {
                return {this->begin() + pos, count};
            }

            return {this->begin() + pos, this->size() - pos};
        }
    };

    template <std::size_t N1, std::size_t N2, typename CharT, typename Traits, typename Allocator>
    static_string<(N1 > N2) ? N1 : N2, CharT, Traits, Allocator>
    operator+(const static_string<N1, CharT, Traits, Allocator> &lhs, const static_string<N2, CharT, Traits, Allocator> &rhs)
    {
        static_string<(N1 > N2) ? N1 : N2, CharT, Traits, Allocator> rv(lhs, lhs.get_allocator());
        rv.append(rhs);
        return rv;
    }

    template <std::size_t N, typename CharT, typename Traits, typename Allocator>
    static_string<N, CharT, Traits, Allocator> operator+(const CharT *lhs, const static_string<N, CharT, Traits, Allocator> &rhs)
    {
        static_string<N, CharT, Traits, Allocator> rv(lhs);
        rv.append(rhs);
        return rv;
    }

    template <std::size_t N, typename CharT, typename Traits, typename Allocator>
    static_string<N, CharT, Traits, Allocator> operator+(CharT lhs, const static_string<N, CharT, Traits, Allocator> &rhs)
    {
        static_string<N, CharT, Traits, Allocator> rv({lhs});
        rv.append(rhs);
        return rv;
    }

    template <std::size_t N, typename CharT, typename Traits, typename Allocator>
    static_string<N, CharT, Traits, Allocator> operator+(const static_string<N, CharT, Traits, Allocator> &lhs, CharT rhs)
    {
        static_string<N, CharT, Traits, Allocator> rv(lhs, lhs.get_allocator());
        rv.push_back(rhs);
        return rv;
    }

    template <std::size_t N1, std::size_t N2, typename CharT, typename Traits, typename Allocator>
    static_string<(N1 > N2) ? N1 : N2, CharT, Traits, Allocator>
    operator+(static_string<N1, CharT, Traits, Allocator> &&lhs, const static_string<N2, CharT, Traits, Allocator> &rhs)
    {
        return std::move(lhs.append(rhs));
    }

    template <std::size_t N1, std::size_t N2, typename CharT, typename Traits, typename Allocator>
    static_string<(N1 > N2) ? N1 : N2, CharT, Traits, Allocator>
    operator+(const static_string<N1, CharT, Traits, Allocator> &lhs, static_string<N2, CharT, Traits, Allocator> &&rhs)
    {
        return std::move(rhs.insert(0, lhs));
    }

    template <std::size_t N1, std::size_t N2, typename CharT, typename Traits, typename Allocator>
    static_string<(N1 > N2) ? N1 : N2, CharT, Traits, Allocator>
    operator+(static_string<N1, CharT, Traits, Allocator> &&lhs, static_string<N2, CharT, Traits, Allocator> &&rhs)
    {
        return std::move(lhs.append(rhs));
    }

    template <std::size_t N, typename CharT, typename Traits, typename Allocator>
    static_string<N, CharT, Traits, Allocator> operator+(const CharT *lhs, static_string<N, CharT, Traits, Allocator> &&rhs)
    {
        return std::move(rhs.insert(0, lhs));
    }

    template <std::size_t N, typename CharT, typename Traits, typename Allocator>
    static_string<N, CharT, Traits, Allocator> operator+(CharT lhs, static_string<N, CharT, Traits, Allocator> &&rhs)
    {
        rhs.insert(rhs.begin(), lhs);
        return std::move(rhs);
    }

    template <std::size_t N, typename CharT, typename Traits, typename Allocator>
    static_string<N, CharT, Traits, Allocator> operator+(static_string<N, CharT, Traits, Allocator> &&lhs, const CharT *rhs)
    {
        return std::move(lhs.append(rhs));
    }

    template <std::size_t N, typename CharT, typename Traits, typename Allocator>
    static_string<N, CharT, Traits, Allocator> operator+(static_string<N, CharT, Traits, Allocator> &&lhs, CharT rhs)
    {
        lhs.push_back(rhs);
        return std::move(lhs);
    }

    template <std::size_t N, typename CharT, typename Traits, typename Allocator>
    std::basic_string<CharT, Traits, Allocator>
    operator+(std::basic_string<CharT, Traits, Allocator> &&lhs, const static_string_impl<CharT, Traits, Allocator> &rhs)
    {
        return std::move(lhs.append(rhs));
    }

    template <std::size_t N, typename CharT, typename Traits, typename Allocator>
    std::basic_string<CharT, Traits, Allocator>
    operator+(const std::basic_string<CharT, Traits, Allocator> &lhs, static_string_impl<CharT, Traits, Allocator> &&rhs)
    {
        return std::move(rhs.insert(0, lhs));
    }

    template <std::size_t N, typename CharT, typename Traits, typename Allocator>
    std::basic_string<CharT, Traits, Allocator>
    operator+(std::basic_string<CharT, Traits, Allocator> &&lhs, static_string_impl<CharT, Traits, Allocator> &&rhs)
    {
        return std::move(lhs.append(rhs));
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator==(const static_string_impl<CharT, Traits, Allocator> &lhs, const static_string_impl<CharT, Traits, Allocator> &rhs)
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        return lhs.compare(rhs) == 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator!=(const static_string_impl<CharT, Traits, Allocator> &lhs, const static_string_impl<CharT, Traits, Allocator> &rhs)
    {
        if (lhs.size() != rhs.size())
        {
            return true;
        }

        return lhs.compare(rhs) != 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator<(const static_string_impl<CharT, Traits, Allocator> &lhs, const static_string_impl<CharT, Traits, Allocator> &rhs)
    {
        return lhs.compare(rhs) < 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator<=(const static_string_impl<CharT, Traits, Allocator> &lhs, const static_string_impl<CharT, Traits, Allocator> &rhs)
    {
        return lhs.compare(rhs) <= 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator>(const static_string_impl<CharT, Traits, Allocator> &lhs, const static_string_impl<CharT, Traits, Allocator> &rhs)
    {
        return lhs.compare(rhs) > 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator>=(const static_string_impl<CharT, Traits, Allocator> &lhs, const static_string_impl<CharT, Traits, Allocator> &rhs)
    {
        return lhs.compare(rhs) >= 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator==(const CharT *lhs, const static_string_impl<CharT, Traits, Allocator> &rhs)
    {
        typename static_string_impl<CharT, Traits, Allocator>::string_view_type sv = lhs;

        if (sv.size() != rhs.size())
        {
            return false;
        }

        return sv.compare(rhs) == 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator==(const static_string_impl<CharT, Traits, Allocator> &lhs, const CharT *rhs)
    {
        typename static_string_impl<CharT, Traits, Allocator>::string_view_type sv = rhs;

        if (lhs.size() != sv.size())
        {
            return false;
        }

        return sv.compare(lhs) == 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator!=(const CharT *lhs, const static_string_impl<CharT, Traits, Allocator> &rhs)
    {
        typename static_string_impl<CharT, Traits, Allocator>::string_view_type sv = lhs;

        if (sv.size() != rhs.size())
        {
            return true;
        }

        return sv.compare(rhs) != 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator!=(const static_string_impl<CharT, Traits, Allocator> &lhs, const CharT *rhs)
    {
        typename static_string_impl<CharT, Traits, Allocator>::string_view_type sv = rhs;

        if (lhs.size() != sv.size())
        {
            return true;
        }

        return sv.compare(lhs) != 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator<(const CharT *lhs, const static_string_impl<CharT, Traits, Allocator> &rhs)
    {
        typename static_string_impl<CharT, Traits, Allocator>::string_view_type sv = lhs;
        return sv.compare(rhs) < 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator<(const static_string_impl<CharT, Traits, Allocator> &lhs, const CharT *rhs)
    {
        typename static_string_impl<CharT, Traits, Allocator>::string_view_type sv = rhs;
        return sv.compare(lhs) > 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator>(const CharT *lhs, const static_string_impl<CharT, Traits, Allocator> &rhs)
    {
        typename static_string_impl<CharT, Traits, Allocator>::string_view_type sv = lhs;
        return sv.compare(rhs) > 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator>(const static_string_impl<CharT, Traits, Allocator> &lhs, const CharT *rhs)
    {
        typename static_string_impl<CharT, Traits, Allocator>::string_view_type sv = rhs;
        return sv.compare(lhs) < 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator<=(const CharT *lhs, const static_string_impl<CharT, Traits, Allocator> &rhs)
    {
        typename static_string_impl<CharT, Traits, Allocator>::string_view_type sv = lhs;
        return sv.compare(rhs) <= 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator<=(const static_string_impl<CharT, Traits, Allocator> &lhs, const CharT *rhs)
    {
        typename static_string_impl<CharT, Traits, Allocator>::string_view_type sv = rhs;
        return sv.compare(lhs) >= 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator>=(const CharT *lhs, const static_string_impl<CharT, Traits, Allocator> &rhs)
    {
        typename static_string_impl<CharT, Traits, Allocator>::string_view_type sv = lhs;
        return sv.compare(rhs) >= 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    bool operator>=(const static_string_impl<CharT, Traits, Allocator> &lhs, const CharT *rhs)
    {
        typename static_string_impl<CharT, Traits, Allocator>::string_view_type sv = rhs;
        return sv.compare(lhs) <= 0;
    }

    template <typename CharT, typename Traits, typename Allocator>
    std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &os, const static_string_impl<CharT, Traits, Allocator> &str);

    template <typename CharT, typename Traits, typename Allocator>
    std::basic_istream<CharT, Traits> &
    operator>>(std::basic_istream<CharT, Traits> &is, const static_string_impl<CharT, Traits, Allocator> &str);

    template <typename CharT, typename Traits, typename Allocator>
    std::basic_istream<CharT, Traits> &
    getline(std::basic_istream<CharT, Traits> &input, static_string_impl<CharT, Traits, Allocator> &str, CharT delim);

    template <typename CharT, typename Traits, typename Allocator>
    std::basic_istream<CharT, Traits> &
    getline(std::basic_istream<CharT, Traits> &&input, static_string_impl<CharT, Traits, Allocator> &str, CharT delim);

    template <typename CharT, typename Traits, typename Allocator>
    std::basic_istream<CharT, Traits> &
    getline(std::basic_istream<CharT, Traits> &input, static_string_impl<CharT, Traits, Allocator> &str);

    template <typename CharT, typename Traits, typename Allocator>
    std::basic_istream<CharT, Traits> &
    getline(std::basic_istream<CharT, Traits> &&input, static_string_impl<CharT, Traits, Allocator> &str);

    template <typename CharT, typename Traits, typename Allocator>
    int stoi(const static_string_impl<CharT, Traits, Allocator> &str, std::size_t *pos = nullptr, int base = 10);

    template <typename CharT, typename Traits, typename Allocator>
    long stol(const static_string_impl<CharT, Traits, Allocator> &str, std::size_t *pos = nullptr, int base = 10);

    template <typename CharT, typename Traits, typename Allocator>
    long long stoll(const static_string_impl<CharT, Traits, Allocator> &str, std::size_t *pos = nullptr, int base = 10);

    template <typename CharT, typename Traits, typename Allocator>
    unsigned long stoul(const static_string_impl<CharT, Traits, Allocator> &str, std::size_t *pos = nullptr, int base = 10);

    template <typename CharT, typename Traits, typename Allocator>
    unsigned long long stoull(const static_string_impl<CharT, Traits, Allocator> &str, std::size_t *pos = nullptr, int base = 10);

    template <typename CharT, typename Traits, typename Allocator>
    float stof(const static_string_impl<CharT, Traits, Allocator> &str, std::size_t *pos = nullptr);

    template <typename CharT, typename Traits, typename Allocator>
    double stod(const static_string_impl<CharT, Traits, Allocator> &str, std::size_t *pos = nullptr);

    template <typename CharT, typename Traits, typename Allocator>
    long double stold(const static_string_impl<CharT, Traits, Allocator> &str, std::size_t *pos = nullptr);

    template <typename CharT, typename Traits, typename Storage, typename Allocator>
    BLESSED_CONSTEXPR_VALUE typename static_string_impl<CharT, Traits, Storage, Allocator>::size_type
        static_string_impl<CharT, Traits, Storage, Allocator>::npos =
            std::numeric_limits<typename static_string_impl<CharT, Traits, Storage, Allocator>::size_type>::max();

#define BLESSED_DETAIL_STRING_HASH                                                        \
    template <typename CharT, typename Traits, typename Allocator>                        \
    struct hash<blessed::static_string_impl<CharT, Traits, Allocator>>                        \
    {                                                                                     \
        using argument_type = blessed::static_string_impl<CharT, Traits, Allocator>;          \
        using result_type = std::size_t;                                                  \
                                                                                          \
        result_type operator()(argument_type const &s) const noexcept                     \
        {                                                                                 \
            using value_type = blessed::remove_pointer_t<decltype(s.data())>;                 \
            return blessed::hash_detail::CityHash{}(s.data(), s.size() * sizeof(value_type)); \
        }                                                                                 \
    };                                                                                    \
                                                                                          \
    template <std::size_t N, typename CharT, typename Traits, typename Allocator>         \
    struct hash<blessed::static_string<N, CharT, Traits, Allocator>>                          \
    {                                                                                     \
        using argument_type = blessed::static_string<N, CharT, Traits, Allocator>;            \
        using result_type = std::size_t;                                                  \
                                                                                          \
        result_type operator()(argument_type const &s) const noexcept                     \
        {                                                                                 \
            using value_type = blessed::remove_pointer_t<decltype(s.data())>;                 \
            return blessed::hash_detail::CityHash{}(s.data(), s.size() * sizeof(value_type)); \
        }                                                                                 \
    };

    BLESSED_DETAIL_STRING_HASH
}

namespace std
{
    template <std::size_t N, typename CharT, typename Traits, typename Allocator>
    void swap(blessed::static_string<N, CharT, Traits, Allocator> &lhs, blessed::static_string<N, CharT, Traits, Allocator> &rhs) noexcept(
        noexcept(lhs.swap(rhs)))
    {
        lhs.swap(rhs);
    }

    BLESSED_DETAIL_STRING_HASH
}

#endif
