
#ifndef BLESSED_STATIC_VECTOR_HPP
#define BLESSED_STATIC_VECTOR_HPP

#include <blessed/blessed.hpp>
#include <blessed/algorithm.hpp>
#include <blessed/detail/vector_storage.hpp>
#include <blessed/memory.hpp>
#include <blessed/type_traits.hpp>
#include <blessed/align.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace blessed
{
    namespace vector_detail
    {
        template <typename Derived, bool Fixed, typename Allocator>
        class vector_base
        {
        private:
            Derived &derived()
            {
                return *static_cast<Derived *>(this);
            }

            const Derived &derived() const
            {
                return *static_cast<const Derived *>(this);
            }

            void deallocate_if_allocated()
            {
                if (derived().storage().get_is_allocated())
                {
                    allocator_traits::deallocate(
                        *derived().storage().get_alloc_ptr(), derived().storage().get_allocated_data(),
                        derived().storage().get_allocated_capacity());
                }
            }

        public:
            using allocator_type = Allocator;
            using allocator_traits = std::allocator_traits<allocator_type>;

            // TODO cleanup duplication
            using value_type = typename allocator_traits::value_type;
            using pointer = typename allocator_traits::pointer;
            using const_pointer = typename allocator_traits::const_pointer;
            using size_type = typename allocator_traits::size_type;
            using difference_type = typename allocator_traits::difference_type;
            using reference = value_type &;
            using const_reference = const value_type &;
            using rvalue_reference = value_type &&;
            using iterator = pointer;
            using const_iterator = const_pointer;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            using move_iterator = std::move_iterator<iterator>;
            using is_memcpy_ok = vector_detail::is_memcpy_ok<allocator_type>;

            template <typename Iterator>
            using iterator_value_adapter = vector_detail::iterator_value_adapter<allocator_type, Iterator>;

            using copy_value_adapter = vector_detail::copy_value_adapter<allocator_type>;
            using default_value_adapter = vector_detail::default_value_adapter<allocator_type>;

            vector_base &operator=(std::initializer_list<value_type> list)
            {
                // static_assert(list.size() < capacity()); TODO enable in C++14
                assign(list.begin(), list.end());

                return *this;
            }

            vector_base &operator=(const vector_base &other)
            {
                if (BLESSED_LIKELY(this != std::addressof(other)))
                {
                    const_pointer other_data = other.derived().data();
                    assign(other_data, other_data + other.derived().size());
                }

                return *this;
            }

            vector_base &operator=(vector_base &&other) noexcept(
                std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value /*||
                std::allocator_traits<Allocator>::is_always_equal::value*/
            )
            {
                if (BLESSED_LIKELY(this != std::addressof(other)))
                {
                    if (is_memcpy_ok::value || other.derived().storage().get_is_allocated())
                    {
                        vector_detail::destroy_elements(derived().storage().get_alloc_ptr(), derived().data(), derived().size());

                        this->derived().storage().deallocate_if_allocated();
                        derived().storage().memcpy_from(other.derived().storage());

                        other.derived().storage().set_inlined_size(0);
                    }
                    else
                    {
                        derived().storage().assign(
                            iterator_value_adapter<move_iterator>(move_iterator(other.derived().storage().get_inlined_data())),
                            other.derived().size());
                    }
                }

                return *this;
            }

            size_type max_size() const noexcept
            {
                return derived().storage().get_is_allocated() ? std::numeric_limits<size_type>::max() / 2
                                                              : derived().storage().get_inlined_capacity();
            }

            BLESSED_NODISCARD bool empty() const noexcept
            {
                return !derived().size();
            }

            reference operator[](size_type i)
            {
                assert(i < derived().size() && "'vector::operator[](size_type)' failed bounds check");
                return derived().data()[i];
            }

            const_reference operator[](size_type i) const
            {
                assert(i < derived().size() && "'vector::operator[](size_type)' failed bounds check");
                return derived().data()[i];
            }

            reference at(size_type i)
            {
#if (__has_feature(cxx_exceptions))
                if (BLESSED_UNLIKELY(i >= derived().size()))
                {
                    throw std::out_of_range("'vector::at(size_type)' failed bounds check");
                }

                return derived().data()[i];
#else
                return operator[](i);
#endif
            }

            const_reference at(size_type i) const
            {
#if (__has_feature(cxx_exceptions))
                if (BLESSED_UNLIKELY(i >= derived().size()))
                {
                    throw std::out_of_range("'vector::at(size_type) const' failed bounds check");
                }

                return derived().data()[i];
#else
                return operator[](i);
#endif
            }

            reference front()
            {
                assert(!empty() && "'vector::front()' vector must not be empty");
                return derived().data()[0];
            }

            const_reference front() const
            {
                assert(!empty() && "'vector::front() const' vector must not be empty");
                return derived().data()[0];
            }

            reference back()
            {
                assert(!empty() && "'vector::back()' vector must not be empty");
                return derived().data()[derived().size() - 1];
            }

            const_reference back() const
            {
                assert(!empty() && "'vector::back() const' vector must not be empty");
                return derived().data()[derived().size() - 1];
            }

            iterator begin() noexcept
            {
                return derived().data();
            }

            const_iterator begin() const noexcept
            {
                return derived().data();
            }

            iterator end() noexcept
            {
                return derived().data() + derived().size();
            }

            const_iterator end() const noexcept
            {
                return derived().data() + derived().size();
            }

            const_iterator cbegin() const noexcept
            {
                return derived().data();
            }

            const_iterator cend() const noexcept
            {
                return derived().data() + derived().size();
            }

            reverse_iterator rbegin() noexcept
            {
                return reverse_iterator(end());
            }

            const_reverse_iterator rbegin() const noexcept
            {
                return const_reverse_iterator(end());
            }

            reverse_iterator rend() noexcept
            {
                return reverse_iterator(begin());
            }

            const_reverse_iterator rend() const noexcept
            {
                return const_reverse_iterator(begin());
            }

            const_reverse_iterator crbegin() const noexcept
            {
                return rbegin();
            }

            const_reverse_iterator crend() const noexcept
            {
                return rend();
            }

            void resize(size_type n)
            {
                assert(n <= max_size() && "'vector::resize(size_type)' n must be less than 'vector::max_size()'");
                derived().storage().resize(default_value_adapter(), n);
            }

            void resize(size_type n, const_reference v)
            {
                assert(n <= max_size() && "'vector::resize(size_type, const_reference)' n must be less than 'vector::max_size()'");
                derived().storage().resize(copy_value_adapter(v), n);
            }

            void assign(size_type n, const_reference v)
            {
                derived().storage().assign(copy_value_adapter(v), n);
            }

            void assign(std::initializer_list<value_type> list)
            {
                // static_assert(list.size() < capacity()); // TODO fix me
                assign(list.begin(), list.end());
            }

            template <typename InputIt>
            auto assign(InputIt first, InputIt last) -> blessed::enable_if_t<
                blessed::iter::is_input_iterator<InputIt>::value && !blessed::iter::is_forward_iterator<InputIt>::value &&
                    std::is_constructible<value_type, typename std::iterator_traits<InputIt>::reference>::value,
                void>;

            template <typename ForwardIt>
            auto assign(ForwardIt first, ForwardIt last) -> blessed::enable_if_t<
                blessed::iter::is_forward_iterator<ForwardIt>::value &&
                    std::is_constructible<value_type, typename std::iterator_traits<ForwardIt>::reference>::value,
                void>;

            auto insert(const_iterator pos, const_reference v) -> iterator;

            auto insert(const_iterator pos, rvalue_reference v) -> iterator;

            auto insert(const_iterator pos, size_type n, const_reference v) -> iterator;

            auto insert(const_iterator pos, std::initializer_list<value_type> list) -> iterator;

            template <typename InputIt>
            auto insert(const_iterator pos, InputIt first, InputIt last) -> blessed::enable_if_t<
                blessed::iter::is_input_iterator<InputIt>::value && !blessed::iter::is_forward_iterator<InputIt>::value &&
                    std::is_constructible<value_type, typename std::iterator_traits<InputIt>::reference>::value,
                iterator>;

            template <typename ForwardIt>
            auto insert(const_iterator pos, ForwardIt first, ForwardIt last) -> blessed::enable_if_t<
                blessed::iter::is_forward_iterator<ForwardIt>::value &&
                    std::is_constructible<value_type, typename std::iterator_traits<ForwardIt>::reference>::value,
                iterator>;

            template <typename... Args>
            auto emplace(const_iterator pos, Args &&...args) -> iterator;

            template <typename... Args>
            auto emplace_back(Args &&...args) -> reference;

            auto push_back(const_reference v) -> void;

            auto push_back(rvalue_reference v) -> void;

            auto pop_back() noexcept -> void;

            auto erase(const_iterator pos) -> iterator;

            auto erase(const_iterator from, const_iterator to) -> iterator;

            auto clear() noexcept -> void;

            auto reserve(size_type n) -> void;

            auto shrink_to_fit() -> void;

            auto swap(vector_base &other) -> void;
        };

        template <typename Derived, bool Fixed, typename Allocator>
        template <typename InputIt>
        auto vector_base<Derived, Fixed, Allocator>::assign(InputIt first, InputIt last) -> blessed::enable_if_t<
            blessed::iter::is_input_iterator<InputIt>::value && !blessed::iter::is_forward_iterator<InputIt>::value &&
                std::is_constructible<value_type, typename std::iterator_traits<InputIt>::reference>::value,
            void>
        {
            clear();

            for (; first != last; ++first)
            {
                emplace_back(*first);
            }
        }

        template <typename Derived, bool Fixed, typename Allocator>
        template <typename ForwardIt>
        auto vector_base<Derived, Fixed, Allocator>::assign(ForwardIt first, ForwardIt last) -> blessed::enable_if_t<
            blessed::iter::is_forward_iterator<ForwardIt>::value &&
                std::is_constructible<value_type, typename std::iterator_traits<ForwardIt>::reference>::value,
            void>
        {
            derived().storage().assign(iterator_value_adapter<ForwardIt>(first), static_cast<size_type>(std::distance(first, last)));
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto vector_base<Derived, Fixed, Allocator>::insert(const_iterator pos, const_reference v) -> iterator
        {
            return emplace(pos, v);
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto vector_base<Derived, Fixed, Allocator>::insert(const_iterator pos, rvalue_reference v) -> iterator
        {
            return emplace(pos, std::move(v));
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto vector_base<Derived, Fixed, Allocator>::insert(const_iterator pos, size_type n, const_reference v) -> iterator
        {
            assert(
                pos >= begin() &&
                "'vector::insert(const_iterator, size_type, const_reference)' pos must not be less than 'vector::begin()'");
            assert(
                pos <= end() &&
                "'vector::insert(const_iterator, size_type, const_reference)' pos must not be greater than 'vector::end()'");

            if (BLESSED_LIKELY(n != 0))
            {
                value_type dealias = v;
                return derived().storage().insert(pos, copy_value_adapter(dealias), n);
            }
            else
            {
                return const_cast<iterator>(pos);
            }
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto vector_base<Derived, Fixed, Allocator>::insert(const_iterator pos, std::initializer_list<value_type> list) -> iterator
        {
            return insert(pos, list.begin(), list.end());
        }

        template <typename Derived, bool Fixed, typename Allocator>
        template <typename InputIt>
        auto vector_base<Derived, Fixed, Allocator>::insert(const_iterator pos, InputIt first, InputIt last) -> blessed::enable_if_t<
            blessed::iter::is_input_iterator<InputIt>::value && !blessed::iter::is_forward_iterator<InputIt>::value &&
                std::is_constructible<value_type, typename std::iterator_traits<InputIt>::reference>::value,
            iterator>
        {
            assert(pos >= begin() && "'vector::insert(const_iterator, InputIt, InputIt)' pos must not be less than 'vector::begin()'");
            assert(pos <= end() && "'vector::insert(const_iterator, InputIt, InputIt)' pos must not be greater than 'vector::end()'");

            size_type index = std::distance(cbegin(), pos);
            for (size_type i = index; first != last; ++i, static_cast<void>(++first))
            {
                insert(derived().data() + i, *first);
            }

            return iterator(derived().data() + index);
        }

        template <typename Derived, bool Fixed, typename Allocator>
        template <typename ForwardIt>
        auto vector_base<Derived, Fixed, Allocator>::insert(const_iterator pos, ForwardIt first, ForwardIt last) -> blessed::enable_if_t<
            blessed::iter::is_forward_iterator<ForwardIt>::value &&
                std::is_constructible<value_type, typename std::iterator_traits<ForwardIt>::reference>::value,
            iterator>
        {
            assert(
                pos >= begin() && "'vector::insert(const_iterator, ForwardIt, ForwardIt)' pos must not be less than 'vector::begin()'");
            assert(pos <= end() && "'vector::insert(const_iterator, ForwardIt, ForwardIt)' pos must not be greater than 'vector::end()'");

            if (BLESSED_LIKELY(first != last))
            {
                return derived().storage().insert(
                    pos, iterator_value_adapter<ForwardIt>(first), static_cast<size_type>(std::distance(first, last)));
            }

            return const_cast<iterator>(pos);
        }

        template <typename Derived, bool Fixed, typename Allocator>
        template <typename... Args>
        auto vector_base<Derived, Fixed, Allocator>::emplace(const_iterator pos, Args &&...args) -> iterator
        {
            assert(pos >= begin() && "'vector::emplace(const_iterator, Args...)' pos must not be less than 'vector::begin()'");
            assert(pos <= end() && "'vector::emplace(const_iterator, Args...)' pos must not be greater than 'vector::end()'");

            value_type dealias(std::forward<Args>(args)...);
            // TODO can this be done without the temp variable
            return derived().storage().insert(pos, iterator_value_adapter<move_iterator>(move_iterator(std::addressof(dealias))), 1);
        }

        template <typename Derived, bool Fixed, typename Allocator>
        template <typename... Args>
        auto vector_base<Derived, Fixed, Allocator>::emplace_back(Args &&...args) -> reference
        {
            return derived().storage().emplace_back(std::forward<Args>(args)...);
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto vector_base<Derived, Fixed, Allocator>::push_back(const_reference v) -> void
        {
            static_cast<void>(emplace_back(v));
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto vector_base<Derived, Fixed, Allocator>::push_back(rvalue_reference v) -> void
        {
            static_cast<void>(emplace_back(std::move(v)));
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto vector_base<Derived, Fixed, Allocator>::pop_back() noexcept -> void
        {
            assert(!empty() && "'vector::pop_back()' vector must not be empty");

            allocator_traits::destroy(*derived().storage().get_alloc_ptr(), derived().data() + (derived().size() - 1));
            derived().storage().subtract_size(1);
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto vector_base<Derived, Fixed, Allocator>::erase(const_iterator pos) -> iterator
        {
            assert(pos >= begin() && "'vector::erase(const_iterator)' pos must not be less than 'vector::begin()'");
            assert(pos <= end() && "'vector::erase(const_iterator)' pos must not be greater than 'vector::end()'");

            return derived().storage().erase(pos, pos + 1);
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto vector_base<Derived, Fixed, Allocator>::erase(const_iterator from, const_iterator to) -> iterator
        {
            assert(from >= begin() && "'vector::erase(const_iterator, const_iterator)' from must not be less than 'vector::begin()'");
            assert(from <= to && "'vector::erase(const_iterator, const_iterator)' from must not be greater than to'");
            assert(to <= end() && "'vector::erase(const_iterator, const_iterator)' to must not be greater than 'vector::end()'");

            if (BLESSED_LIKELY(from != to))
            {
                return derived().storage().erase(from, to);
            }

            return const_cast<iterator>(from);
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto vector_base<Derived, Fixed, Allocator>::clear() noexcept -> void
        {
            vector_detail::destroy_elements(derived().storage().get_alloc_ptr(), derived().data(), derived().size());

            if (derived().inline_capacity() > 0)
            {
                derived().storage().deallocate_if_allocated();
                derived().storage().set_inlined_size(0);
            }
            else
            {
                derived().storage().set_size(0);
            }
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto vector_base<Derived, Fixed, Allocator>::reserve(size_type n) -> void
        {
            derived().storage().reserve(n);
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto vector_base<Derived, Fixed, Allocator>::shrink_to_fit() -> void
        {
            if (derived().storage().get_is_allocated())
            {
                derived().storage().shrink_to_fit();
            }
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto vector_base<Derived, Fixed, Allocator>::swap(vector_base &other) -> void
        {
            if (BLESSED_LIKELY(
                    this != std::addressof(
                                other))) // TODO I think it should be possible to avoid this check, as std::swap(a, a) should be a no-op
            {
                derived().storage().swap(std::addressof(other.derived().storage()));
            }
        }

        template <typename T, bool Fixed, typename Allocator>
        class vector_ref : public vector_base<vector_ref<T, Fixed, Allocator>, Fixed, Allocator>
        {
            friend class vector_base<vector_ref<T, Fixed, Allocator>, Fixed, Allocator>;

            using base = vector_base<vector_ref<T, Fixed, Allocator>, Fixed, Allocator>;

        public:
            using storage_type = vector_detail::storage_ref<T, Fixed, Allocator>;
            using allocator_type = typename storage_type::allocator_type;

            using size_type = typename storage_type::size_type;
            using difference_type = typename storage_type::difference_type;
            using value_type = typename storage_type::value_type;

            using pointer = typename storage_type::pointer;
            using const_pointer = typename storage_type::const_pointer;

            using reference = typename storage_type::reference;
            using const_reference = typename storage_type::const_reference;

            using iterator = typename storage_type::iterator;
            using const_iterator = typename storage_type::const_iterator;

            using reverse_iterator = typename storage_type::reverse_iterator;
            using const_reverse_iterator = typename storage_type::const_reverse_iterator;

            vector_ref(const vector_ref &other) : vector_ref(other, *other.storage().get_alloc_ptr()) {}

            vector_ref(const vector_ref &other, const allocator_type &alloc) : storage_ref_(other.storage(), alloc) {}

            vector_ref(vector_ref &&other) noexcept(std::is_nothrow_move_constructible<value_type>::value)
                : storage_ref_(std::move(other.storage()))
            {
            }

            vector_ref(vector_ref &&other, const allocator_type &alloc) noexcept(std::is_nothrow_move_constructible<value_type>::value)
                : storage_ref_(std::move(other.storage()), alloc)
            {
            }

            vector_ref(const storage_type &other) : storage_ref_(other) {} // TODO private?

            ~vector_ref() = default;

            vector_ref &operator=(std::initializer_list<value_type> list)
            {
                base::operator=(list);

                return *this;
            }

            vector_ref &operator=(const vector_ref &other)
            {
                base::operator=(other);

                return *this;
            }

            vector_ref &operator=(vector_ref &&other) noexcept(
                std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
                std::allocator_traits<Allocator>::is_always_equal::value)
            {
                base::operator=(other);

                return *this;
            }

            size_type size() const noexcept
            {
                return storage().get_size();
            }

            size_type capacity() const noexcept
            {
                return storage().get_is_allocated() ? storage().get_allocated_capacity() : storage().get_inlined_capacity();
            }

            size_type inline_capacity() const noexcept
            {
                return storage().get_inlined_capacity();
            }

            pointer data() noexcept
            {
                return storage().get_is_allocated() ? storage().get_allocated_data() : storage().get_inlined_data();
            }

            const_pointer data() const noexcept
            {
                return storage().get_is_allocated() ? storage().get_allocated_data() : storage().get_inlined_data();
            }

            allocator_type get_allocator() const
            {
                return *storage().get_alloc_ptr();
            }

        private:
            storage_type &storage()
            {
                return storage_ref_;
            }

            const storage_type &storage() const
            {
                return storage_ref_;
            }

            storage_type storage_ref_;
        };

        template <typename T, std::size_t N, bool Fixed, typename Allocator = std::allocator<T>>
        class vector : public vector_base<vector<T, N, Fixed, Allocator>, Fixed, Allocator>
        {
            friend class vector_base<vector<T, N, Fixed, Allocator>, Fixed, Allocator>;

            using base = vector_base<vector<T, N, Fixed, Allocator>, Fixed, Allocator>;
            using allocator_traits = std::allocator_traits<Allocator>;

        public:
            using storage_type = vector_detail::storage<T, N, Fixed, Allocator>;
            using allocator_type = typename storage_type::allocator_type;

            using size_type = typename storage_type::size_type;
            using difference_type = typename storage_type::difference_type;
            using value_type = typename storage_type::value_type;

            using pointer = typename storage_type::pointer;
            using const_pointer = typename storage_type::const_pointer;

            using reference = typename storage_type::reference;
            using const_reference = typename storage_type::const_reference;

            using iterator = typename storage_type::iterator;
            using const_iterator = typename storage_type::const_iterator;

            using reverse_iterator = typename storage_type::reverse_iterator;
            using const_reverse_iterator = typename storage_type::const_reverse_iterator;

            using ref_type = vector_ref<T, Fixed, Allocator>;

            // TODO remove?
            template <typename U>
            struct rebind
            {
            private:
                using alloc_other = typename allocator_traits::template rebind_alloc<U>;

            public:
                using other = vector<U, N, Fixed, alloc_other>;
            };

            vector() noexcept(noexcept(allocator_type())) : storage_() {}

            explicit vector(const allocator_type &alloc) noexcept : storage_(alloc) {}

            explicit vector(size_type n, const allocator_type &alloc = allocator_type()) : storage_(n, alloc) {}

            vector(size_type n, const_reference v, const allocator_type &alloc = allocator_type()) : storage_(n, v, alloc) {}

            vector(std::initializer_list<value_type> list, const allocator_type &alloc = allocator_type())
                : vector(list.begin(), list.end(), alloc)
            {
            }

            template <typename InputIt>
            vector(
                InputIt first,
                InputIt last,
                const allocator_type &alloc = allocator_type(),
                blessed::enable_if_t<
                    blessed::iter::is_input_iterator<InputIt>::value &&
                    std::is_constructible<value_type, typename std::iterator_traits<InputIt>::reference>::value> * /*unused*/
                = nullptr)
                : storage_(first, last, alloc)
            {
            }

            vector(const vector &other) : vector(other, *other.storage_.get_alloc_ptr()) {}

            vector(const vector &other, const allocator_type &alloc) : storage_(other.storage_, alloc) {}

            vector(vector &&other) noexcept(std::is_nothrow_move_constructible<value_type>::value) : storage_(std::move(other.storage_))
            {
            }

            vector(vector &&other, const allocator_type &alloc) noexcept(std::is_nothrow_move_constructible<value_type>::value)
                : storage_(std::move(other.storage_), alloc)
            {
            }

            ~vector() = default;

            vector &operator=(std::initializer_list<value_type> list)
            {
                base::operator=(list);

                return *this;
            }

            vector &operator=(const vector &other)
            {
                base::operator=(other);

                return *this;
            }

            vector &operator=(vector &&other) noexcept(
                std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value /*||
                std::allocator_traits<Allocator>::is_always_equal::value*/
            )
            {
                base::operator=(std::move(other));

                return *this;
            }

            operator vector_ref<T, Fixed, Allocator>()
            {
                return vector_ref<T, Fixed, Allocator>(storage_);
            }

            size_type size() const noexcept
            {
                return storage_.get_size();
            }

            size_type capacity() const noexcept
            {
                return storage_.get_is_allocated() ? storage_.get_allocated_capacity() : storage_.get_inlined_capacity();
            }

            size_type inline_capacity() const noexcept
            {
                return storage_.get_inlined_capacity();
            }

            pointer data() noexcept
            {
                return storage_.get_is_allocated() ? storage_.get_allocated_data() : storage_.get_inlined_data();
            }

            const_pointer data() const noexcept
            {
                return storage_.get_is_allocated() ? storage_.get_allocated_data() : storage_.get_inlined_data();
            }

            allocator_type get_allocator() const
            {
                return *storage_.get_alloc_ptr();
            }

        private:
            storage_type &storage()
            {
                return storage_;
            }

            const storage_type &storage() const
            {
                return storage_;
            }

            storage_type storage_;
        };

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        bool operator==(const vector<T, N, Fixed, Allocator> &a, const vector<T, N, Fixed, Allocator> &b)
        {
#ifdef BLESSED_HAS_ROBUST_NONMODIFYING_SEQ_OPS
            return std::equal(a.data(), a.data() + a.size(), b.data(), b.data() + b.size());
#else
            return (a.size() == b.size()) && std::equal(a.data(), a.data() + a.size(), b.data());
#endif
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        bool operator!=(const vector<T, N, Fixed, Allocator> &a, const vector<T, N, Fixed, Allocator> &b)
        {
            return !(a == b);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        bool operator<(const vector<T, N, Fixed, Allocator> &a, const vector<T, N, Fixed, Allocator> &b)
        {
            return std::lexicographical_compare(a.data(), a.data() + a.size(), b.data(), b.data() + b.size());
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        bool operator>(const vector<T, N, Fixed, Allocator> &a, const vector<T, N, Fixed, Allocator> &b)
        {
            return b < a;
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        bool operator<=(const vector<T, N, Fixed, Allocator> &a, const vector<T, N, Fixed, Allocator> &b)
        {
            return !(b < a);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        bool operator>=(const vector<T, N, Fixed, Allocator> &a, const vector<T, N, Fixed, Allocator> &b)
        {
            return !(a < b);
        }

        template <typename T, bool Fixed, typename Allocator>
        bool operator==(const vector_ref<T, Fixed, Allocator> &a, const vector_ref<T, Fixed, Allocator> &b)
        {
#ifdef BLESSED_HAS_ROBUST_NONMODIFYING_SEQ_OPS
            return std::equal(a.data(), a.data() + a.size(), b.data(), b.data() + b.size());
#else
            return (a.size() == b.size()) && std::equal(a.data(), a.data() + a.size(), b.data());
#endif
        }

        template <typename T, bool Fixed, typename Allocator>
        bool operator!=(const vector_ref<T, Fixed, Allocator> &a, const vector_ref<T, Fixed, Allocator> &b)
        {
            return !(a == b);
        }

        template <typename T, bool Fixed, typename Allocator>
        bool operator<(const vector_ref<T, Fixed, Allocator> &a, const vector_ref<T, Fixed, Allocator> &b)
        {
            return std::lexicographical_compare(a.data(), a.data() + a.size(), b.data(), b.data() + b.size());
        }

        template <typename T, bool Fixed, typename Allocator>
        bool operator>(const vector_ref<T, Fixed, Allocator> &a, const vector_ref<T, Fixed, Allocator> &b)
        {
            return b < a;
        }

        template <typename T, bool Fixed, typename Allocator>
        bool operator<=(const vector_ref<T, Fixed, Allocator> &a, const vector_ref<T, Fixed, Allocator> &b)
        {
            return !(b < a);
        }

        template <typename T, bool Fixed, typename Allocator>
        bool operator>=(const vector_ref<T, Fixed, Allocator> &a, const vector_ref<T, Fixed, Allocator> &b)
        {
            return !(a < b);
        }
    }

    template <typename T, size_t N, typename Allocator = std::allocator<T>>
    using static_stack_vector = vector_detail::vector<T, N, true, Allocator>;

    template <typename T, size_t N = 0, typename Allocator = std::allocator<T>>
    using static_heap_vector = vector_detail::vector<T, 0, true, Allocator>;

    template <typename T, size_t N, typename Allocator = std::allocator<T>>
    using dyn_vector = vector_detail::vector<T, N, false, Allocator>;

    template <typename T, bool Fixed, typename Allocator>
    using vector_ref = blessed::vector_detail::vector_ref<T, Fixed, Allocator>;
}

namespace std
{
    template <typename T, size_t N, bool Fixed, typename Storage>
    inline void swap(
        blessed::vector_detail::vector<T, N, Fixed, Storage> &lhs,
        blessed::vector_detail::vector<T, N, Fixed, Storage> &rhs) noexcept(noexcept(lhs.swap(rhs)))
    {
        lhs.swap(rhs);
    }

    template <typename T, bool Fixed, typename Storage>
    inline void swap(
        blessed::vector_detail::vector_ref<T, Fixed, Storage> &lhs,
        blessed::vector_detail::vector_ref<T, Fixed, Storage> &rhs) noexcept(noexcept(lhs.swap(rhs)))
    {
        lhs.swap(rhs);
    }
}

#endif
