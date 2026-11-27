
#ifndef BLESSED_DETAIL_STATIC_VECTOR_IMPL_HPP
#define BLESSED_DETAIL_STATIC_VECTOR_IMPL_HPP

#include <blessed/blessed.hpp>
#include <blessed/align.hpp>

#include <cassert>

namespace blessed
{
    namespace detail
    {
        template <typename Allocator>
        class static_vector_base
        {
        protected:
            std::size_t size_{0};
            std::size_t capacity_;
            BLESSED_NO_UNIQUE_ADDRESS Allocator allocator_;

            static_vector_base() = delete;
            explicit static_vector_base(std::size_t cap, const Allocator &a) : capacity_(cap), allocator_(a) {}

            Allocator &get_base_allocator()
            {
                return allocator_;
            }

            const Allocator &get_base_allocator() const
            {
                return allocator_;
            }

        public:
            Allocator &get_allocator() const
            {
                return allocator_;
            }

            std::size_t size() const
            {
                return size_;
            }

            std::size_t capacity() const
            {
                return capacity_;
            }

            BLESSED_NODISCARD bool empty() const
            {
                return !size_;
            }

            void set_size(std::size_t n)
            {
                assert(n <= capacity());
                size_ = n;
            }
        };

        template <typename T, typename Allocator>
        struct static_vector_storage_alignment_and_size
        {
            blessed::aligned_storage<static_vector_base<Allocator>> base_;
            blessed::aligned_storage<T> storage_;
        };

        template <typename T, std::size_t N>
        struct static_vector_storage
        {
            blessed::aligned_storage<std::array<T, N>> storage_;

            void *get_first_element()
            {
                return &storage_.buffer;
            }

            const void *get_first_element() const
            {
                return &storage_.buffer;
            }
        };

        /*
         * This is the part of small_vector_template_base which does not depend on whether the type T is a POD.
         */
        template <typename T, typename Allocator, typename Storage>
        class static_vector_template_common : public static_vector_base<Allocator>
        {
            Storage storage_;

        protected:
            static_vector_template_common(std::size_t n, const Allocator &a) : static_vector_base<Allocator>(n, a) {}

            Allocator &get_allocator_ref()
            {
                return this->get_base_allocator();
            }

            const Allocator &get_allocator_ref() const
            {
                return this->get_base_allocator();
            }

        public:
            using allocator_type = Allocator;

            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            using value_type = T;

            using pointer = T *;
            using const_pointer = const T *;

            using reference = T &;
            using const_reference = const T &;

            using iterator = pointer;
            using const_iterator = const_pointer;

            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            iterator begin()
            {
                return static_cast<iterator>(storage_.get_first_element());
            }

            const_iterator begin() const
            {
                return static_cast<const_iterator>(storage_.get_first_element());
            }

            const_iterator cbegin() const
            {
                return static_cast<const_iterator>(storage_.get_first_element());
            }

            iterator end()
            {
                return static_cast<iterator>(begin() + this->size());
            }

            const_iterator end() const
            {
                return static_cast<const_iterator>(begin() + this->size());
            }

            const_iterator cend() const
            {
                return static_cast<const_iterator>(cbegin() + this->size());
            }

            reverse_iterator rbegin()
            {
                return reverse_iterator(end());
            }

            const_reverse_iterator rbegin() const
            {
                return const_reverse_iterator(end());
            }

            const_reverse_iterator crbegin() const
            {
                return const_reverse_iterator(cend());
            }

            reverse_iterator rend()
            {
                return reverse_iterator(begin());
            }

            const_reverse_iterator rend() const
            {
                return const_reverse_iterator(begin());
            }

            const_reverse_iterator crend() const
            {
                return const_reverse_iterator(cbegin());
            }

            size_type max_size() const
            {
                return this->capacity();
            }

            pointer data()
            {
                return static_cast<pointer>(begin());
            }

            const_pointer data() const
            {
                return static_cast<const_pointer>(begin());
            }

            reference operator[](size_type idx)
            {
                assert(idx < this->size());
                return begin()[idx];
            }

            const_reference operator[](size_type idx) const
            {
                assert(idx < this->size());
                return begin()[idx];
            }

            reference at(size_type idx)
            {
#if (__has_feature(cxx_exceptions))
                if (idx < this->size())
                {
                    return operator[](idx);
                }
                else
                {
                    throw std::out_of_range("static_vector::at() out of range");
                }
#else
                return operator[](idx);
#endif
            }

            const_reference at(size_type idx) const
            {
#if (__has_feature(cxx_exceptions))
                if (idx < this->size())
                {
                    return operator[](idx);
                }
                else
                {
                    throw std::out_of_range("static_vector::at() out of range");
                }
#else
                return operator[](idx);
#endif
            }

            reference front()
            {
                assert(!this->empty());
                return begin()[0];
            }

            const_reference front() const
            {
                assert(!this->empty());
                return begin()[0];
            }

            reference back()
            {
                assert(!this->empty());
                return end()[-1];
            }

            const_reference back() const
            {
                assert(!this->empty());
                return end()[-1];
            }
        };

        template <typename T, typename Allocator, typename Storage, bool = blessed::is_trivially_copyable<T>::value>
        class static_vector_template_base : public static_vector_template_common<T, Allocator, Storage>
        {
            using alloc_traits = std::allocator_traits<Allocator>;

        protected:
            static_vector_template_base(std::size_t n, const Allocator &a) : static_vector_template_common<T, Allocator, Storage>(n, a) {}

            void destroy_range(T *first, T *last)
            {
                while (first != last)
                {
                    --last;
                    alloc_traits::destroy(this->get_allocator_ref(), last);
                }
            }

            template <typename It1, typename It2>
            static void uninitialized_move(It1 first, It1 last, It2 dest)
            {
                std::uninitialized_copy(std::make_move_iterator(first), std::make_move_iterator(last), dest);
            }

            template <typename It1, typename It2>
            static void uninitialized_copy(It1 first, It1 last, It2 dest)
            {
                std::uninitialized_copy(first, last, dest);
            }

        public:
            void push_back(const T &val)
            {
                assert(this->size() < this->capacity());
                alloc_traits::construct(this->get_allocator_ref(), this->end(), val);
                this->set_size(this->size() + 1);
            }

            void push_back(T &&val)
            {
                assert(this->size() < this->capacity());
                alloc_traits::construct(this->get_allocator_ref(), this->end(), ::std::move(val));
                this->set_size(this->size() + 1);
            }

            void pop_back()
            {
                this->set_size(this->size() - 1);
                alloc_traits::destroy(this->get_allocator_ref(), this->end());
            }
        };

        /*
         * SmallVectorTemplateBase<TriviallyCopyable = true> - This is where we put
         * method implementations that are designed to work with POD-like T's.
         */
        template <typename T, typename Allocator, typename Storage>
        class static_vector_template_base<T, Allocator, Storage, true> : public static_vector_template_common<T, Allocator, Storage>
        {
            using alloc_traits = std::allocator_traits<Allocator>;

        protected:
            static_vector_template_base(std::size_t n, const Allocator &a) : static_vector_template_common<T, Allocator, Storage>(n, a) {}

            void destroy_range(T *first, T *last)
            {
                while (first != last)
                {
                    --last;
                    alloc_traits::destroy(this->get_allocator_ref(), last);
                }
            }

            template <typename It1, typename It2>
            static void uninitialized_move(It1 first, It1 last, It2 dest)
            {
                std::uninitialized_copy(std::make_move_iterator(first), std::make_move_iterator(last), dest);
            }

            template <typename It1, typename It2>
            static void uninitialized_copy(It1 first, It1 last, It2 dest)
            {
                std::uninitialized_copy(first, last, dest);
            }

        public:
            void push_back(const T &val)
            {
                assert(this->size() < this->capacity());
                alloc_traits::construct(this->get_allocator_ref(), this->end(), val);
                this->set_size(this->size() + 1);
            }

            void push_back(T &&val)
            {
                assert(this->size() < this->capacity());
                alloc_traits::construct(this->get_allocator_ref(), this->end(), ::std::move(val));
                this->set_size(this->size() + 1);
            }

            void pop_back()
            {
                this->set_size(this->size() - 1);
                alloc_traits::destroy(this->get_allocator_ref(), this->end());
            }
        };

    } // namespace detail

    template <typename T, typename Storage, typename Allocator = std::allocator<T>>
    class static_vector_impl : public detail::static_vector_template_base<T, Allocator, Storage>
    {
        using super = detail::static_vector_template_base<T, Allocator, Storage>;

        using alloc_traits = std::allocator_traits<Allocator>;

    public:
        using iterator = typename super::iterator;
        using const_iterator = typename super::const_iterator;
        using reference = typename super::reference;
        using size_type = typename super::size_type;

    protected:
        explicit static_vector_impl(std::size_t n, const Allocator &a)
            : detail::static_vector_template_base<T, Allocator, Storage>(n, a)
        {
        }

    public:
        static_vector_impl(const static_vector_impl &) = delete;

        static_vector_impl &operator=(const static_vector_impl &rhs)
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

                // destroy excess elements
                this->destroy_range(new_end, this->end());

                // trim
                this->set_size(rhs_size);
                return *this;
            }

            if (this->capacity() < rhs.size())
            {
                assert(0 && "Cannot assign static_vector_impl not enough space");
            }
            else if (cur_size)
            {
                // Otherwise, use assignment for the already-constructed elements.
                std::copy(rhs.begin(), rhs.begin() + cur_size, this->begin());
            }

            // Copy construct the new elements in place.
            this->uninitialized_copy(rhs.begin() + cur_size, rhs.end(), this->begin() + cur_size);
            this->set_size(rhs_size);

            return *this;
        }

        static_vector_impl &operator=(static_vector_impl &&rhs) noexcept
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

                // destroy excess elements
                this->destroy_range(new_end, this->end());
                this->set_size(rhs_size);

                rhs.clear();

                return *this;
            }

            if (this->capacity() < rhs.size())
            {
                assert(0 && "Cannot assign static_vector_impl not enough space");
            }
            else if (cur_size)
            {
                // Otherwise, use assignment for the already-constructed elements.
                std::move(rhs.begin(), rhs.begin() + cur_size, this->begin());
            }

            // Copy construct the new elements in place.
            this->uninitialized_move(rhs.begin() + cur_size, rhs.end(), this->begin() + cur_size);
            this->set_size(rhs_size);

            rhs.clear();

            return *this;
        }

        ~static_vector_impl() = default;

        void clear()
        {
            this->destroy_range(this->begin(), this->end());
            this->set_size(0);
        }

        void resize(size_type n)
        {
            if (n < this->size())
            {
                this->destroy_range(this->begin() + n, this->end());
                this->set_size(n);
            }
            else if (n > this->size())
            {
                for (iterator it = this->begin() + this->size(), end_it = this->begin() + this->capacity(); it != end_it; ++it)
                {
                    alloc_traits::construct(this->get_allocator_ref(), &*it, T());
                }

                this->set_size(n);
            }
        }

        /**
         * Resize to n, and overwrite data via `size_type op(T *data, size_type n, allocator_type& alloc)` function,
         *   return value number of Ts written.
         *
         * NOTE: this may change in the future to mirror any std::vector changes to keep api compatibility
         */
        template <typename Operation>
        void resize_and_overwrite(size_type n, Operation op)
        {
            // NOTE: in vectors with changable capacity this may need to grow the buffer
            assert(n <= this->max_size() && "static_vector_impl::resize_and_overwrite() out of range");

            auto m = static_cast<size_type>(op(this->data(), n, this->get_allocator_ref()));

            assert(m <= n && "op() must not right more than n characters");

            if (n < this->size())
            {
                this->destroy_range(this->begin() + n, this->end());
            }

            this->set_size(m);
        }

        void swap(static_vector_impl &rhs)
        {
            if (this == &rhs)
            {
                return;
            }

            assert(rhs.size() <= this->capacity() && "static_vector_impl::swap() rhs does not fit");
            assert(this->size() <= rhs.capacity() && "static_vector_impl::swap() lhs does not fit");

            // swap the shared elements
            size_type n_shared = std::min(this->size(), rhs.size());

            for (size_type i = 0; i != n_shared; ++i)
            {
                std::swap((*this)[i], rhs[i]);
            }

            // TODO can this be move?
            // copy over the extra
            if (this->size() > rhs.size())
            {
                size_type n_diff = this->size() - rhs.size();
                this->uninitialized_copy(this->begin() + n_shared, this->end(), rhs.end());
                rhs.set_size(rhs.size() + n_diff);
                this->destroy_range(this->begin() + n_shared, this->end());
                this->set_size(n_shared);
            }
            else if (rhs.size() > this->size())
            {
                size_type n_diff = rhs.size() - this->size();
                this->uninitialized_copy(rhs.begin() + n_shared, rhs.end(), this->end());
                this->set_size(this->size() + n_diff);
                this->destroy_range(rhs.begin() + n_shared, rhs.end());
                rhs.set_size(n_shared);
            }
        }

        template <
            typename InputIter,
            typename = blessed::enable_if_t<
                std::is_convertible<typename std::iterator_traits<InputIter>::iterator_category, std::input_iterator_tag>::value>>
        void append(InputIter first, InputIter last)
        {
            size_type n = (size_t)std::distance(first, last);

            if (n > this->capacity() - this->size())
            {
                assert(0 && "Not enough space to assign");
            }

            this->uninitialized_copy(first, last, this->end());
            this->set_size(this->size() + n);
        }

        void append(size_type n, const T &val)
        {
            if (this->capacity() < n)
            {
                assert(0 && "Not enough space to assign");
            }

            std::uninitialized_fill_n(this->end(), n, val);
            this->set_size(this->size() + n);
        }

        void append(std::initializer_list<T> il)
        {
            append(il.begin(), il.end());
        }

        void assign(size_type n, const T &val)
        {
            clear();

            if (this->capacity() < n)
            {
                assert(0 && "Not enough space to assign");
            }

            this->set_size(n);
            std::uninitialized_fill(this->begin(), this->end(), val);
        }

        template <
            typename InputIter,
            typename = blessed::enable_if_t<
                std::is_convertible<typename std::iterator_traits<InputIter>::iterator_category, std::input_iterator_tag>::value>>
        void assign(InputIter first, InputIter last)
        {
            clear();
            append(first, last);
        }

        void assign(std::initializer_list<T> il)
        {
            clear();
            append(il);
        }

        iterator erase(const_iterator it)
        {
            assert(this->size() > 0 && "Size must be positive");
            assert(it >= this->begin() && it < this->end() && "pos must be within the vector");

            iterator p = this->begin() + (it - this->cbegin());

            // shift all the element down one
            std::move(p + 1, this->end(), p);

            // drop the last element
            this->pop_back();

            return p;
        }

        iterator erase(const_iterator cfirst, const_iterator clast)
        {
            iterator first = this->begin() + (cfirst - this->cbegin());
            iterator last = this->begin() + (clast - this->cbegin());

            assert(first >= this->begin() && "Range to erase is out of bounds");
            assert(first <= last && "Must be a valid range");
            assert(last <= this->end() && "Trying to erase must the end");

            iterator p = first;
            iterator it = std::move(last, this->end(), first);

            this->destroy_range(it, this->end());
            this->set_size(static_cast<size_t>(it - this->begin()));

            return p;
        }

        iterator insert(const_iterator it, T &&val)
        {
            if (it == this->cend())
            {
                this->push_back(::std::move(val));
                return this->end() - 1;
            }

            assert(it >= this->cbegin() && "Insertion iterator is out of bounds");
            assert(it <= this->cend() && "Insertion past the end of the vector");

            if (this->size() >= this->capacity())
            {
                assert(0 && "not enough space to insert");
            }

            alloc_traits::construct(this->get_allocator_ref(), this->end(), ::std::move(this->back()));

            // push everything else over
            iterator p = this->begin() + (it - this->cbegin());
            std::move_backward(p, this->end() - 1, this->end());
            this->set_size(this->size() + 1);

            // If we just moved the element we're inserting, be sure to update the reference
            T *val_ptr = &val;
            if (it <= val_ptr && val_ptr < this->end())
            {
                ++val_ptr;
            }

            *p = ::std::move(*val_ptr);

            return p;
        }

        iterator insert(const_iterator it, const T &val)
        {
            if (it == this->cend())
            {
                this->push_back(val);
                return this->end() - 1;
            }

            assert(it >= this->cbegin() && "Insertion iterator is out of bounds");
            assert(it <= this->cend() && "Insertion past the end of the vector");

            if (this->size() >= this->capacity())
            {
                assert(0 && "not enough space to insert");
            }

            alloc_traits::construct(this->get_allocator_ref(), this->end(), ::std::move(this->back()));

            // push everything else over
            iterator p = this->begin() + (it - this->cbegin());
            std::move_backward(p, this->end() - 1, this->end());
            this->set_size(this->size() + 1);

            // If we just moved the element we're inserting, be sure to update the reference
            const T *val_ptr = &val;
            if (p <= val_ptr && val_ptr < this->end())
            {
                ++val_ptr;
            }

            *p = *val_ptr;

            return p;
        }

        iterator insert(const_iterator it, size_type n, const T &val)
        {
            auto insert_num = it - this->begin();

            if (it == this->cend())
            {
                append(n, val);
                return this->begin() + insert_num;
            }

            assert(it >= this->begin() && "Insertion iterator is out of bounds");
            assert(it <= this->end() && "Inserting past the end of the vector");
            assert((this->size() + n) <= this->capacity() && "Not enough space");

            it = this->begin() + insert_num;

            iterator p = this->begin() + (it - this->cbegin());

            // If there are more elements between the insertion point and the end of the
            // range than there are being inserted, we can use a simple approach to
            // insertion.
            if ((size_t)(this->cend() - p) >= n)
            {
                T *old_end = this->end();

                append(std::move_iterator<iterator>(this->end() - n), std::move_iterator<iterator>(this->end()));

                // copy the existing elements that get replaced
                std::copy_backward(p, old_end - n, old_end);

                std::fill_n(p, n, val);
                return p;
            }

            // Otherwise, we're inserting more elements than exist already, and we're
            // not inserting at the end.

            // Move over the elements that we're about to overwrite.
            T *old_end = this->end();
            this->set_size(this->size() + n);
            auto num_overwritten = old_end - p;
            this->uninitialized_move(p, old_end, this->end() - num_overwritten);

            // replace the overwritten part
            std::fill_n(p, num_overwritten, val);

            // insert the non-overwritten middle part
            std::uninitialized_fill_n(old_end, n - (size_t)num_overwritten, val);
            return p;
        }

        template <
            typename InputIter,
            typename = blessed::enable_if_t<
                std::is_convertible<typename std::iterator_traits<InputIter>::iterator_category, std::input_iterator_tag>::value>>
        iterator insert(const_iterator it, InputIter first, InputIter last)
        {
            auto insert_num = it - this->begin();

            if (it == this->end())
            {
                append(first, last);
                return this->begin() + insert_num;
            }

            assert(it >= this->begin() && "Insertion iterator is out of bounds");
            assert(it <= this->end() && "Inserting past the end of the vector");

            auto num_to_insert = std::distance(first, last);

            assert((this->size() + (std::size_t)num_to_insert) <= this->capacity() && "Not enough space");

            it = this->begin() + insert_num;

            iterator p = this->begin() + (it - this->cbegin());

            // If there are more elements between the insertion point and the end of the
            // range than there are being inserted, we can use a simple approach to
            // insertion.
            if (this->end() - p >= num_to_insert)
            {
                T *old_end = this->end();
                append(std::move_iterator<iterator>(this->end() - num_to_insert), std::move_iterator<iterator>(this->end()));

                // copy the existing elements that get replaced
                std::move_backward(p, old_end - num_to_insert, old_end);

                std::copy(first, last, p);
                return p;
            }

            // Otherwise, we're insert more elements than exist already, and we're not inserting at the end.
            T *old_end = this->end();
            this->set_size(this->size() + (std::size_t)num_to_insert);
            auto num_overwritten = old_end - p;
            this->uninitialized_move(p, old_end, this->end() - num_overwritten);

            // replace the overwritten part
            for (auto i = p; num_overwritten > 0; --num_overwritten)
            {
                *i = *first;
                ++i;
                ++first;
            }

            this->uninitialized_copy(first, last, old_end);
            return p;
        }

        iterator insert(const_iterator it, std::initializer_list<T> il)
        {
            return insert(it, il.begin(), il.end());
        }

        template <typename... Args>
        iterator emplace(const_iterator it, Args &&...args)
        {
            assert(this->size() < this->capacity() && "vector must not be full");

            iterator p = this->begin() + (it - this->cbegin());

            if (p == this->end())
            {
                alloc_traits::construct(this->get_allocator_ref(), this->end(), std::forward<Args>(args)...);
                this->set_size(this->size() + 1);
            }
            else
            {
                alloc_traits::construct(this->get_allocator_ref(), this->end(), ::std::move(this->back()));
                std::move_backward(p, this->end() - 1, this->end());
                alloc_traits::construct(this->get_allocator_ref(), p, std::forward<Args>(args)...);
                this->set_size(this->size() + 1);
            }

            return p;
        }

        template <typename... Args>
        reference emplace_back(Args &&...args)
        {
            assert(this->size() < this->capacity() && "vector must not be full");
            alloc_traits::construct(this->get_allocator_ref(), this->end(), std::forward<Args>(args)...);
            this->set_size(this->size() + 1);
            return this->back();
        }

        bool operator==(const static_vector_impl &rhs) const
        {
#ifdef BLESSED_HAS_ROBUST_NONMODIFYING_SEQ_OPS
            return std::equal(this->cbegin(), this->cend(), rhs.cbegin(), rhs.cend());
#else
            return (this->size() == rhs.size()) && std::equal(this->cbegin(), this->cend(), rhs.cbegin());
#endif
        }

        bool operator!=(const static_vector_impl &rhs) const
        {
            return !(*this == rhs);
        }

        bool operator<(const static_vector_impl &rhs) const
        {
            return std::lexicographical_compare(this->cbegin(), this->cend(), rhs.cbegin(), rhs.cend());
        }

        bool operator<=(const static_vector_impl &rhs) const
        {
            return !(rhs < *this);
        }

        bool operator>(const static_vector_impl &rhs) const
        {
            return rhs < *this;
        }

        bool operator>=(const static_vector_impl &rhs) const
        {
            return !(*this < rhs);
        }
    };
}

#endif
