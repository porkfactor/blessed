
#include <blessed/iterator.hpp>
#include <blessed/span.hpp>
#include <blessed/type_traits.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>

namespace blessed
{
    namespace vector_detail
    {
        template <typename Allocator, typename T = typename std::allocator_traits<Allocator>::value_type>
        using is_memcpy_ok = blessed::conjunction<
            std::is_same<Allocator, std::allocator<T>>,
            blessed::is_trivially_copy_constructible<T>,
            blessed::is_trivially_copy_assignable<T>,
            blessed::is_trivially_destructible<T>>;

        template <typename Allocator, typename Pointer, typename SizeType>
        void destroy_elements(Allocator *alloc_ptr, Pointer destroy_first, SizeType destroy_size)
        {
            using allocator_traits = std::allocator_traits<Allocator>;

            if (destroy_first)
            {
                for (auto i = destroy_size; i != 0;)
                {
                    --i;
                    allocator_traits::destroy(*alloc_ptr, destroy_first + i);
                }

#if !defined(NDEBUG)
                using value_type = typename allocator_traits::value_type;

                void *memory_ptr = destroy_first;
                auto memory_size = destroy_size * sizeof(value_type);
                std::memset(memory_ptr, 0xca, memory_size);
#endif
            }
        }

        template <typename Allocator, typename Pointer, typename ValueAdapter, typename SizeType>
        void construct_elements(Allocator *alloc_ptr, Pointer construct_first, ValueAdapter *values_ptr, SizeType construct_size)
        {
            for (SizeType i = 0; i < construct_size; ++i)
            {
                BLESSED_INTERNAL_TRY
                {
                    values_ptr->construct_next(alloc_ptr, construct_first + i);
                }

                BLESSED_INTERNAL_CATCH_ANY
                {
                    destroy_elements(alloc_ptr, construct_first, i);
                    BLESSED_INTERNAL_RETHROW;
                }
            }
        }

        template <typename Pointer, typename ValueAdapter, typename SizeType>
        void assign_elements(Pointer assign_first, ValueAdapter *values_ptr, SizeType assign_size)
        {
            for (SizeType i = 0; i < assign_size; ++i)
            {
                values_ptr->assign_next(assign_first + i);
            }
        }

        template <typename Allocator>
        struct storage_view
        {
            using allocator_traits = std::allocator_traits<Allocator>;
            using pointer = typename allocator_traits::pointer;
            using size_type = typename allocator_traits::size_type;

            pointer data;
            size_type size;
            size_type capacity;
        };

        template <typename Allocator, typename Iterator>
        class iterator_value_adapter
        {
            using allocator_traits = std::allocator_traits<Allocator>;
            using pointer = typename allocator_traits::pointer;

        public:
            explicit iterator_value_adapter(const Iterator &it) : it_(it) {}

            void construct_next(Allocator *alloc_ptr, pointer construct_at)
            {
                allocator_traits::construct(*alloc_ptr, construct_at, *it_);
                ++it_;
            }

            void assign_next(pointer assign_at)
            {
                *assign_at = *it_;
                ++it_;
            }

        private:
            Iterator it_;
        };

        template <typename Allocator>
        class copy_value_adapter
        {
            using allocator_traits = std::allocator_traits<Allocator>;
            using value_type = typename allocator_traits::value_type;
            using pointer = typename allocator_traits::pointer;
            using const_pointer = typename allocator_traits::const_pointer;

        public:
            explicit copy_value_adapter(const value_type &v) : ptr_(std::addressof(v)) {}

            void construct_next(Allocator *alloc_ptr, pointer construct_at)
            {
                allocator_traits::construct(*alloc_ptr, construct_at, *ptr_);
            }

            void assign_next(pointer assign_at)
            {
                *assign_at = *ptr_;
            }

        private:
            const_pointer ptr_;
        };

        template <typename Allocator>
        class default_value_adapter
        {
            using allocator_traits = std::allocator_traits<Allocator>;
            using value_type = typename allocator_traits::value_type;
            using pointer = typename allocator_traits::pointer;

        public:
            explicit default_value_adapter() = default;

            void construct_next(Allocator *alloc_ptr, pointer construct_at)
            {
                allocator_traits::construct(*alloc_ptr, construct_at);
            }

            void assign_next(pointer assign_at)
            {
                *assign_at = value_type();
            }
        };

        template <typename Allocator>
        class allocation_transaction
        {
            using allocator_traits = std::allocator_traits<Allocator>;
            using pointer = typename allocator_traits::pointer;
            using size_type = typename allocator_traits::size_type;

        public:
            explicit allocation_transaction(Allocator *alloc_ptr) : allocator_(*alloc_ptr), data_(nullptr) {}

            ~allocation_transaction()
            {
                if (did_allocate())
                {
                    allocator_traits::deallocate(allocator_, data_, capacity_);
                }
            }

            allocation_transaction(const allocation_transaction &) = delete;
            void operator=(const allocation_transaction &) = delete;

            Allocator &get_allocator()
            {
                return allocator_;
            }

            pointer &get_data()
            {
                return data_;
            }

            size_type &get_capacity()
            {
                return capacity_;
            }

            bool did_allocate()
            {
                return data_;
            }

            pointer allocate(size_type capacity)
            {
                get_data() = allocator_traits::allocate(get_allocator(), capacity);
                get_capacity() = capacity;
                return get_data();
            }

            void reset()
            {
                get_data() = nullptr;
                get_capacity() = 0;
            }

        private:
            BLESSED_NO_UNIQUE_ADDRESS Allocator allocator_;
            pointer data_;
            size_type capacity_ = 0;
        };

        template <typename Allocator>
        class construction_transaction
        {
            using allocator_traits = std::allocator_traits<Allocator>;
            using pointer = typename allocator_traits::pointer;
            using size_type = typename allocator_traits::size_type;

        public:
            explicit construction_transaction(Allocator *alloc_ptr) : allocator_(*alloc_ptr), data_(nullptr) {}

            ~construction_transaction()
            {
                if (did_construct())
                {
                    destroy_elements(std::addressof(allocator_), get_data(), get_size());
                }
            }

            construction_transaction(const construction_transaction &) = delete;
            void operator=(const construction_transaction &) = delete;

            Allocator &get_allocator()
            {
                return allocator_;
            }

            pointer &get_data()
            {
                return data_;
            }

            size_type &get_size()
            {
                return size_;
            }

            bool did_construct()
            {
                return data_ != nullptr;
            }

            template <typename ValueAdapter>
            void construct(pointer data, ValueAdapter *values_ptr, size_type size)
            {
                construct_elements(std::addressof(allocator_), data, values_ptr, size);
                get_data() = data;
                get_size() = size;
            }

            void commit()
            {
                get_data() = nullptr;
                get_size() = 0;
            }

        private:
            BLESSED_NO_UNIQUE_ADDRESS Allocator allocator_;
            pointer data_;
            size_type size_ = 0;
        };

        template <typename SizeType, typename Allocator>
        struct data_members
        {
            data_members(const Allocator &allocator, SizeType size) : allocator_(allocator), size_(size) {}

            BLESSED_NO_UNIQUE_ADDRESS Allocator allocator_;
            SizeType size_;
        };

        template <typename Ptr, typename SizeType>
        struct allocated
        {
            Ptr allocated_data_;
            SizeType allocated_capacity_;
        };

        template <typename T, std::size_t InlineSize>
        struct inlined
        {
            alignas(T) char inlined_data_[sizeof(T[InlineSize])];
        };

        template <typename T>
        struct inlined<T, 0>
        {
        };

        template <typename Derived, bool Fixed, typename Allocator>
        class storage_base
        {
        private:
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
            using storage_view = vector_detail::storage_view<allocator_type>;

            template <typename Iterator>
            using iterator_value_adapter = vector_detail::iterator_value_adapter<allocator_type, Iterator>;

            using copy_value_adapter = vector_detail::copy_value_adapter<allocator_type>;
            using default_value_adapter = vector_detail::default_value_adapter<allocator_type>;
            using allocation_transaction = vector_detail::allocation_transaction<allocator_type>;
            using construction_transaction = vector_detail::construction_transaction<allocator_type>;

            using is_memcpy_ok = vector_detail::is_memcpy_ok<allocator_type>;

            Derived &derived()
            {
                return *static_cast<Derived *>(this);
            }

            const Derived &derived() const
            {
                return *static_cast<const Derived *>(this);
            }

            static auto next_capacity(size_type current_capacity) -> size_type;

            static auto compute_capacity(size_type current_capacity, size_type requested_capacity) -> size_type;

            auto make_storage_view() -> storage_view;

            auto acquire_allocated_data(allocation_transaction *allocation_tx_ptr) -> void;

            template <typename... Args>
            BLESSED_NEVER_INLINE auto emplace_back_slow(Args &&...args) -> reference;

        public:
            auto deallocate_if_allocated() -> void;

            template <typename ValueAdapter>
            auto initialize(ValueAdapter values, size_type new_size, size_type new_capacity) -> void;

            template <typename ValueAdapter>
            auto assign(ValueAdapter values, size_type new_size) -> void;

            template <typename ValueAdapter>
            auto resize(ValueAdapter values, size_type new_size) -> void;

            // TODO add resize_and_overwrite()

            // TODO emplace() without using move construction/assignment

            template <typename ValueAdapter>
            auto insert(const_iterator pos, ValueAdapter values, size_type insert_count) -> iterator;

            template <typename... Args, bool U = Fixed>
            auto emplace_back(Args &&...args) -> blessed::enable_if_t<U, reference>;

            template <typename... Args, bool U = Fixed>
            auto emplace_back(Args &&...args) -> blessed::enable_if_t<!U, reference>;

            auto erase(const_iterator from, const_iterator to) -> iterator;

            auto reserve(size_type requested_capacity) -> void;

            auto shrink_to_fit() -> void;

            auto swap(storage_base *other_storage_ptr) -> void;
        };

        template <typename Derived, bool Fixed, typename Allocator>
        auto storage_base<Derived, Fixed, Allocator>::next_capacity(size_type current_capacity) -> size_type
        {
            return current_capacity * 2;
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto storage_base<Derived, Fixed, Allocator>::compute_capacity(size_type current_capacity, size_type requested_capacity)
            -> size_type
        {
            return std::max(next_capacity(current_capacity), requested_capacity);
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto storage_base<Derived, Fixed, Allocator>::make_storage_view() -> storage_view
        {
            return derived().get_is_allocated()
                       ? storage_view{derived().get_allocated_data(), derived().get_size(), derived().get_allocated_capacity()}
                       : storage_view{derived().get_inlined_data(), derived().get_size(), derived().get_inlined_capacity()};
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto storage_base<Derived, Fixed, Allocator>::acquire_allocated_data(allocation_transaction *allocation_tx_ptr) -> void
        {
            derived().set_allocated_data(allocation_tx_ptr->get_data(), allocation_tx_ptr->get_capacity());

            allocation_tx_ptr->reset();
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto storage_base<Derived, Fixed, Allocator>::deallocate_if_allocated() -> void
        {
            if (derived().get_is_allocated())
            {
                allocator_traits::deallocate(
                    *derived().get_alloc_ptr(), derived().get_allocated_data(), derived().get_allocated_capacity());
            }
        }

        template <typename Derived, bool Fixed, typename Allocator>
        template <typename ValueAdapter>
        auto storage_base<Derived, Fixed, Allocator>::initialize(ValueAdapter values, size_type new_size, size_type new_capacity) -> void
        {
            assert(!derived().get_is_allocated());
            assert(derived().get_size() == 0);

            pointer construct_data;
            if (new_capacity > derived().get_inlined_capacity())
            {
                construct_data = allocator_traits::allocate(*derived().get_alloc_ptr(), new_capacity);
                derived().set_allocated_data(construct_data, new_capacity);
                derived().set_is_allocated();
            }
            else
            {
                construct_data = derived().get_inlined_data();
                derived().unset_is_allocated();
            }

            construct_elements(derived().get_alloc_ptr(), construct_data, &values, new_size);

            derived().add_size(new_size);
        }

        template <typename Derived, bool Fixed, typename Allocator>
        template <typename ValueAdapter>
        auto storage_base<Derived, Fixed, Allocator>::assign(ValueAdapter values, size_type new_size) -> void
        {
            auto storage = make_storage_view();

            allocation_transaction allocation_tx(derived().get_alloc_ptr());

            blessed::span<value_type> assign_loop;
            blessed::span<value_type> construct_loop;
            blessed::span<value_type> destroy_loop;

            if (new_size > storage.capacity)
            {
                size_type new_capacity = compute_capacity(storage.capacity, new_size);
                construct_loop = {allocation_tx.allocate(new_capacity), new_size};
                destroy_loop = {storage.data, storage.size};
            }
            else if (new_size > storage.size)
            {
                assign_loop = {storage.data, storage.size};
                construct_loop = {storage.data + storage.size, new_size - storage.size};
            }
            else
            {
                assign_loop = {storage.data, new_size};
                destroy_loop = {storage.data + new_size, storage.size - new_size};
            }

            vector_detail::assign_elements(assign_loop.data(), &values, assign_loop.size());
            vector_detail::construct_elements(derived().get_alloc_ptr(), construct_loop.data(), &values, construct_loop.size());
            vector_detail::destroy_elements(derived().get_alloc_ptr(), destroy_loop.data(), destroy_loop.size());

            if (allocation_tx.did_allocate())
            {
                deallocate_if_allocated();
                acquire_allocated_data(&allocation_tx);
                derived().set_is_allocated();
            }

            derived().set_size(new_size);
        }

        template <typename Derived, bool Fixed, typename Allocator>
        template <typename ValueAdapter>
        auto storage_base<Derived, Fixed, Allocator>::resize(ValueAdapter values, size_type new_size) -> void
        {
            auto storage = make_storage_view();

            iterator_value_adapter<move_iterator> move_values(move_iterator(storage.data));

            allocation_transaction allocation_tx(derived().get_alloc_ptr());
            construction_transaction construction_tx(derived().get_alloc_ptr());

            blessed::span<value_type> construct_loop;
            blessed::span<value_type> move_construct_loop;
            blessed::span<value_type> destroy_loop;

            if (new_size > storage.capacity)
            {
                size_type new_capacity = compute_capacity(storage.capacity, new_size);
                pointer new_data = allocation_tx.allocate(new_capacity);
                construct_loop = {new_data + storage.size, new_size - storage.size};
                move_construct_loop = {new_data, storage.size};
                destroy_loop = {storage.data, storage.size};
            }
            else if (new_size > storage.size)
            {
                construct_loop = {storage.data + storage.size, new_size - storage.size};
            }
            else
            {
                destroy_loop = {storage.data + new_size, storage.size - new_size};
            }

            construction_tx.construct(construct_loop.data(), &values, construct_loop.size());

            vector_detail::construct_elements(
                derived().get_alloc_ptr(), move_construct_loop.data(), &move_values, move_construct_loop.size());
            vector_detail::destroy_elements(derived().get_alloc_ptr(), destroy_loop.data(), destroy_loop.size());

            construction_tx.commit();
            if (allocation_tx.did_allocate())
            {
                deallocate_if_allocated();
                acquire_allocated_data(&allocation_tx);
                derived().set_is_allocated();
            }

            derived().set_size(new_size);
        }

        // TODO split into smaller functions
        template <typename Derived, bool Fixed, typename Allocator>
        template <typename ValueAdapter>
        auto storage_base<Derived, Fixed, Allocator>::insert(const_iterator pos, ValueAdapter values, size_type insert_count) -> iterator
        {
            auto storage = make_storage_view();

            auto const insert_index = static_cast<size_type>(std::distance(const_iterator(storage.data), pos));
            size_type const insert_end_index = insert_index + insert_count;
            size_type const new_size = storage.size + insert_count;

            if (new_size > storage.capacity)
            {
                if (Fixed)
                {
                    // TODO throw exception if exceptions enabled
                    assert(0 && "Exceeded capacity on fixed sized vector");
                }

                allocation_transaction allocation_tx(derived().get_alloc_ptr());
                construction_transaction construction_tx(derived().get_alloc_ptr());
                construction_transaction move_construction_tx(derived().get_alloc_ptr());

                iterator_value_adapter<move_iterator> move_values(move_iterator(storage.data));

                size_type new_capacity = compute_capacity(storage.capacity, new_size);
                pointer new_data = allocation_tx.allocate(new_capacity);

                construction_tx.construct(new_data + insert_index, &values, insert_count);
                move_construction_tx.construct(new_data, &move_values, insert_index);

                vector_detail::construct_elements(
                    derived().get_alloc_ptr(), new_data + insert_end_index, &move_values, storage.size - insert_index);
                vector_detail::destroy_elements(derived().get_alloc_ptr(), storage.data, storage.size);

                construction_tx.commit();
                move_construction_tx.commit();
                deallocate_if_allocated();
                acquire_allocated_data(&allocation_tx);

                derived().set_allocated_size(new_size);

                return iterator(new_data + insert_index);
            }

            size_type move_construction_destination_index = std::max(insert_end_index, storage.size);

            construction_transaction move_construction_tx(derived().get_alloc_ptr());

            iterator_value_adapter<move_iterator> move_construction_values(
                move_iterator(storage.data + (move_construction_destination_index - insert_count)));

            blessed::span<value_type> move_construction = {storage.data + move_construction_destination_index,
                                                       new_size - move_construction_destination_index};

            pointer move_assignment_values = storage.data + insert_index;
            blessed::span<value_type> move_assignment = {storage.data + insert_end_index,
                                                     move_construction_destination_index - insert_end_index};

            blessed::span<value_type> insert_assignment = {move_assignment_values, move_construction.size()};
            blessed::span<value_type> insert_construction = {insert_assignment.data() + insert_assignment.size(),
                                                         insert_count - insert_assignment.size()};

            move_construction_tx.construct(move_construction.data(), &move_construction_values, move_construction.size());

            for (pointer destination = move_assignment.data() + move_assignment.size(), last_destination = move_assignment.data(),
                         source = move_assignment_values + move_assignment.size();
                 ;)
            {
                --destination;
                --source;

                if (destination < last_destination)
                {
                    break;
                }

                *destination = std::move(*source);
            }

            vector_detail::assign_elements(insert_assignment.data(), &values, insert_assignment.size());
            vector_detail::construct_elements(derived().get_alloc_ptr(), insert_construction.data(), &values, insert_construction.size());

            move_construction_tx.commit();

            derived().add_size(insert_count);
            return iterator(storage.data + insert_index);
        }

        template <typename Derived, bool Fixed, typename Allocator>
        template <typename... Args, bool U>
        auto storage_base<Derived, Fixed, Allocator>::emplace_back(Args &&...args) -> blessed::enable_if_t<U, reference>
        {
            auto storage = make_storage_view();

            size_type const n = storage.size;

            assert(n != storage.capacity);

            pointer last_ptr = storage.data + n;

            allocator_traits::construct(*derived().get_alloc_ptr(), last_ptr, std::forward<Args>(args)...);

            derived().add_size(1);

            return *last_ptr;
        }

        template <typename Derived, bool Fixed, typename Allocator>
        template <typename... Args, bool U>
        auto storage_base<Derived, Fixed, Allocator>::emplace_back(Args &&...args) -> blessed::enable_if_t<!U, reference>
        {
            auto storage = make_storage_view();

            size_type const n = storage.size;

            if (BLESSED_LIKELY(n != storage.capacity))
            {
                pointer last_ptr = storage.data + n;

                allocator_traits::construct(*derived().get_alloc_ptr(), last_ptr, std::forward<Args>(args)...);

                derived().add_size(1);

                return *last_ptr;
            }

            return emplace_back_slow(std::forward<Args>(args)...);
        }

        template <typename Derived, bool Fixed, typename Allocator>
        template <typename... Args>
        auto storage_base<Derived, Fixed, Allocator>::emplace_back_slow(Args &&...args) -> reference
        {
            auto storage = make_storage_view();

            if (Fixed)
            {
                // TODO throw exception if exceptions enabled
                assert(0 && "Exceeded capacity on fixed sized vector");
            }

            allocation_transaction allocation_tx(derived().get_alloc_ptr());
            iterator_value_adapter<move_iterator> move_values(move_iterator(storage.data));

            pointer construct_data = storage.data;
            size_type new_capacity = next_capacity(storage.capacity);
            construct_data = allocation_tx.allocate(new_capacity);
            pointer last_ptr = construct_data + storage.size;

            allocator_traits::construct(*derived().get_alloc_ptr(), last_ptr, std::forward<Args>(args)...);

            BLESSED_INTERNAL_TRY
            {
                vector_detail::construct_elements(derived().get_alloc_ptr(), allocation_tx.get_data(), &move_values, storage.size);
            }
            BLESSED_INTERNAL_CATCH_ANY
            {
                allocator_traits::destroy(*derived().get_alloc_ptr(), last_ptr);
                BLESSED_INTERNAL_RETHROW;
            }

            vector_detail::destroy_elements(derived().get_alloc_ptr(), storage.data, storage.size);

            deallocate_if_allocated();
            acquire_allocated_data(&allocation_tx);
            derived().set_is_allocated();

            derived().add_size(1);
            return *last_ptr;
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto storage_base<Derived, Fixed, Allocator>::erase(const_iterator from, const_iterator to) -> iterator
        {
            auto storage = make_storage_view();

            auto erase_size = static_cast<size_type>(std::distance(from, to));
            auto erase_index = static_cast<size_type>(std::distance(const_iterator(storage.data), from));
            size_type erase_end_index = erase_index + erase_size;

            iterator_value_adapter<move_iterator> move_values(move_iterator(storage.data + erase_end_index));

            vector_detail::assign_elements(storage.data + erase_index, &move_values, storage.size - erase_end_index);
            vector_detail::destroy_elements(derived().get_alloc_ptr(), storage.data + (storage.size - erase_size), erase_size);

            derived().subtract_size(erase_size);
            return iterator(storage.data + erase_index);
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto storage_base<Derived, Fixed, Allocator>::reserve(size_type requested_capacity) -> void
        {
            auto storage = make_storage_view();

            if (BLESSED_UNLIKELY(requested_capacity <= storage.capacity))
            {
                return;
            }

            allocation_transaction allocation_tx(derived().get_alloc_ptr());
            iterator_value_adapter<move_iterator> move_values(move_iterator(storage.data));

            pointer new_data = allocation_tx.allocate(requested_capacity);

            vector_detail::construct_elements(derived().get_alloc_ptr(), new_data, &move_values, storage.size);
            vector_detail::destroy_elements(derived().get_alloc_ptr(), storage.data, storage.size);

            deallocate_if_allocated();
            acquire_allocated_data(&allocation_tx);
            derived().set_is_allocated();
        }

        template <typename Derived, bool Fixed, typename Allocator>
        auto storage_base<Derived, Fixed, Allocator>::shrink_to_fit() -> void
        {
            assert(derived().get_is_allocated() && "can only be called on an allocated instance");

            storage_view storage{derived().get_allocated_data(), derived().get_size(), derived().get_allocated_capacity()};

            if (BLESSED_UNLIKELY(storage.size == storage.capacity))
            {
                return;
            }

            allocation_transaction allocation_tx(derived().get_alloc_ptr());
            iterator_value_adapter<move_iterator> move_values(move_iterator(storage.data));

            pointer construct_data = derived().get_inlined_data();
            if (storage.size > derived().get_inlined_capacity())
            {
                size_type new_capacity = storage.size;
                construct_data = allocation_tx.allocate(new_capacity);
            }

            BLESSED_INTERNAL_TRY
            {
                vector_detail::construct_elements(derived().get_alloc_ptr(), construct_data, &move_values, storage.size);
            }
            BLESSED_INTERNAL_CATCH_ANY
            {
                derived().set_allocated_data(storage.data, storage.capacity);
                BLESSED_INTERNAL_RETHROW;
            }

            vector_detail::destroy_elements(derived().get_alloc_ptr(), storage.data, storage.size);

            if (allocation_tx.did_allocate())
            {
                acquire_allocated_data(&allocation_tx);
            }
            else
            {
                derived().unset_is_allocated();
            }
        }

        // TODO split into smaller functions
        template <typename Derived, bool Fixed, typename Allocator>
        auto storage_base<Derived, Fixed, Allocator>::swap(storage_base *other_storage_ptr) -> void
        {
            using std::swap;
            assert(this != other_storage_ptr);

            if (derived().get_is_allocated() && other_storage_ptr->derived().get_is_allocated())
            {
                swap(derived().get_allocated_data(), other_storage_ptr->derived().get_allocated_data());
                derived().swap_allocated_capacity(other_storage_ptr->derived());
            }
            else if (!derived().get_is_allocated() && !other_storage_ptr->derived().get_is_allocated())
            {
                Derived *small_ptr = std::addressof(derived());
                Derived *large_ptr = std::addressof(other_storage_ptr->derived());

                if (small_ptr->get_size() > large_ptr->get_size())
                {
                    swap(small_ptr, large_ptr);
                }

                for (size_type i = 0; i < small_ptr->get_size(); ++i)
                {
                    swap(small_ptr->get_inlined_data()[i], large_ptr->get_inlined_data()[i]);
                }

                iterator_value_adapter<move_iterator> move_values(move_iterator(large_ptr->get_inlined_data() + small_ptr->get_size()));

                vector_detail::construct_elements(
                    large_ptr->get_alloc_ptr(), small_ptr->get_inlined_data() + small_ptr->get_size(), &move_values,
                    large_ptr->get_size() - small_ptr->get_size());

                vector_detail::destroy_elements(
                    large_ptr->get_alloc_ptr(), large_ptr->get_inlined_data() + small_ptr->get_size(),
                    large_ptr->get_size() - small_ptr->get_size());
            }
            else
            {
                Derived *allocated_ptr = std::addressof(derived());
                Derived *inlined_ptr = std::addressof(other_storage_ptr->derived());

                if (!allocated_ptr->get_is_allocated())
                {
                    swap(allocated_ptr, inlined_ptr);
                }

                storage_view allocated_storage_view{allocated_ptr->get_allocated_data(), allocated_ptr->get_size(),
                                                    allocated_ptr->get_allocated_capacity()};

                iterator_value_adapter<move_iterator> move_values(move_iterator(inlined_ptr->get_inlined_data()));

                BLESSED_INTERNAL_TRY
                {
                    vector_detail::construct_elements(
                        inlined_ptr->get_alloc_ptr(), allocated_ptr->get_inlined_data(), &move_values, inlined_ptr->get_size());
                }
                BLESSED_INTERNAL_CATCH_ANY
                {
                    allocated_ptr->set_allocated_data(allocated_storage_view.data, allocated_storage_view.capacity);
                    BLESSED_INTERNAL_RETHROW;
                }

                vector_detail::destroy_elements(inlined_ptr->get_alloc_ptr(), inlined_ptr->get_inlined_data(), inlined_ptr->get_size());

                inlined_ptr->set_allocated_data(allocated_storage_view.data, allocated_storage_view.capacity);
            }

            derived().swap_size(other_storage_ptr->derived());
            swap(*derived().get_alloc_ptr(), *other_storage_ptr->derived().get_alloc_ptr());
        }

        // TODO reduce duplication between storage_ref and storage
        template <typename T, bool Fixed, typename Allocator>
        class storage_ref : public storage_base<storage_ref<T, Fixed, Allocator>, Fixed, Allocator>
        {
            friend class storage_base<storage_ref<T, Fixed, Allocator>, Fixed, Allocator>;

        public:
            using allocator_traits = std::allocator_traits<Allocator>;
            using allocator_type = typename allocator_traits::allocator_type;
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

            using is_memcpy_ok = vector_detail::is_memcpy_ok<allocator_type>;

            storage_ref(
                data_members<size_type, allocator_type> &members,
                size_type small_size,
                allocated<pointer, size_type> &allocated,
                pointer inlined_data);

            storage_ref(const storage_ref &other);

            auto get_size() const -> size_type;

            auto get_capacity() const -> size_type;

            auto swap_size(storage_ref &other_storage) -> void;

            auto get_is_allocated() const -> bool;

            auto get_allocated_data() -> pointer &;

            auto get_allocated_data() const -> const_pointer;

            auto get_inlined_data() -> pointer;

            auto get_inlined_data() const -> const_pointer;

            auto get_allocated_capacity() const -> size_type;

            auto swap_allocated_capacity(storage_ref &other_storage) -> void;

            auto get_inlined_capacity() const -> size_type;

            auto get_alloc_ptr() -> allocator_type *;

            auto get_alloc_ptr() const -> const allocator_type *;

            auto set_is_allocated() -> void;

            auto unset_is_allocated() -> void;

            auto set_size(size_type size) -> void;

            auto set_allocated_size(size_type size) -> void;

            auto set_inlined_size(size_type size) -> void;

            auto add_size(size_type count) -> void;

            auto subtract_size(size_type count) -> void;

            auto set_allocated_data(pointer ptr, size_type capacity) -> void;

            auto memcpy_from(const storage_ref &other_storage) -> void;

            auto get_storage() -> storage_ref &;

            auto get_storage() const -> const storage_ref &;

        private:
            data_members<size_type, allocator_type> &members_;
            const size_type small_size_; // capacity for Fixed types

            // TODO union?
            allocated<pointer, size_type> &allocated_;
            pointer inlined_data_;
        };

        template <typename T, bool Fixed, typename Allocator>
        storage_ref<T, Fixed, Allocator>::storage_ref(
            data_members<size_type, allocator_type> &members,
            size_type small_size,
            allocated<pointer, size_type> &allocated,
            pointer inlined_data)
            : members_(members), small_size_(small_size), allocated_(allocated), inlined_data_(inlined_data)
        {
        }

        template <typename T, bool Fixed, typename Allocator>
        storage_ref<T, Fixed, Allocator>::storage_ref(const storage_ref &other)
            : storage_ref(other.members_, other.small_size_, other.allocated_, other.inlined_data_)
        {
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::get_size() const -> size_type
        {
            return members_.size_ >> 1;
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::get_capacity() const -> size_type
        {
            return get_is_allocated() ? get_allocated_capacity() : get_inlined_capacity();
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::swap_size(storage_ref &other_storage) -> void
        {
            using std::swap;
            swap(members_.size_, other_storage.members_.size_);
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::get_is_allocated() const -> bool
        {
            return members_.size_ & 1;
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::get_allocated_data() -> pointer &
        {
            return get_is_allocated() ? allocated_.allocated_data_ : inlined_data_;
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::get_allocated_data() const -> const_pointer
        {
            return get_is_allocated() ? allocated_.allocated_data_ : inlined_data_;
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::get_inlined_data() -> pointer
        {
            return inlined_data_;
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::get_inlined_data() const -> const_pointer
        {
            return inlined_data_;
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::get_allocated_capacity() const -> size_type
        {
            return allocated_.allocated_capacity_;
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::swap_allocated_capacity(storage_ref &other_storage) -> void
        {
            using std::swap;
            swap(allocated_.allocated_capacity_, other_storage.allocated_.allocated_capacity_);
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::get_inlined_capacity() const -> size_type
        {
            return small_size_;
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::get_alloc_ptr() -> allocator_type *
        {
            return std::addressof(members_.allocator_);
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::get_alloc_ptr() const -> const allocator_type *
        {
            return std::addressof(members_.allocator_);
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::set_is_allocated() -> void
        {
            members_.size_ |= static_cast<size_type>(1);
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::unset_is_allocated() -> void
        {
            members_.size_ &= (std::numeric_limits<size_type>::max() - 1);
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::set_size(size_type size) -> void
        {
            members_.size_ = (size << 1) | static_cast<size_type>(get_is_allocated());
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::set_allocated_size(size_type size) -> void
        {
            members_.size_ = (size << 1) | static_cast<size_type>(1);
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::set_inlined_size(size_type size) -> void
        {
            members_.size_ = size << static_cast<size_type>(1);
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::add_size(size_type count) -> void
        {
            members_.size_ += count << static_cast<size_type>(1);
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::subtract_size(size_type count) -> void
        {
            assert(count <= get_size());

            members_.size_ -= count << static_cast<size_type>(1);
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::set_allocated_data(pointer ptr, size_type capacity) -> void
        {
            allocated_.allocated_data_ = ptr;
            allocated_.allocated_capacity_ = capacity;
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::memcpy_from(const storage_ref &other_storage) -> void
        {
            assert(is_memcpy_ok::value || other_storage.get_is_allocated());

            members_.size_ = other_storage.members_.size_;
            allocated_ = other_storage.allocated_;
            inlined_data_ = other_storage.inlined_data_;
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::get_storage() -> storage_ref &
        {
            return *this;
        }

        template <typename T, bool Fixed, typename Allocator>
        auto storage_ref<T, Fixed, Allocator>::get_storage() const -> const storage_ref &
        {
            return *this;
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        class storage : public storage_base<storage<T, N, Fixed, Allocator>, Fixed, Allocator>
        {
            friend class storage_base<storage<T, N, Fixed, Allocator>, Fixed, Allocator>;
            using base = storage_base<storage<T, N, Fixed, Allocator>, Fixed, Allocator>;

        public:
            using allocator_traits = std::allocator_traits<Allocator>;
            using allocator_type = typename allocator_traits::allocator_type;
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

            using is_memcpy_ok = vector_detail::is_memcpy_ok<allocator_type>;

            using move_iterator = std::move_iterator<iterator>;

            template <typename It>
            using iterator_value_adapter = vector_detail::iterator_value_adapter<allocator_type, It>;

            using copy_value_adapter = vector_detail::copy_value_adapter<allocator_type>;
            using default_value_adapter = vector_detail::default_value_adapter<allocator_type>;

            explicit storage(const allocator_type &alloc = allocator_type());

            explicit storage(size_type n, const allocator_type &alloc = allocator_type());

            storage(size_type n, const_reference v, const allocator_type &alloc = allocator_type());

            template <typename InputIt>
            storage(
                InputIt first,
                InputIt last,
                const allocator_type &alloc = allocator_type(),
                blessed::enable_if_t<
                    blessed::iter::is_input_iterator<InputIt>::value && !blessed::iter::is_forward_iterator<InputIt>::value &&
                    std::is_constructible<value_type, typename std::iterator_traits<InputIt>::reference>::value> * /*unused*/
                = nullptr);

            template <typename ForwardIt>
            storage(
                ForwardIt first,
                ForwardIt last,
                const allocator_type &alloc = allocator_type(),
                blessed::enable_if_t<
                    blessed::iter::is_forward_iterator<ForwardIt>::value &&
                    std::is_constructible<value_type, typename std::iterator_traits<ForwardIt>::reference>::value> * /*unused*/
                = nullptr);

            storage(const storage &other, const allocator_type &alloc);

            storage(storage &&other) noexcept(std::is_nothrow_move_constructible<value_type>::value);

            storage(storage &&other, const allocator_type &alloc) noexcept(std::is_nothrow_move_constructible<value_type>::value);

            ~storage();

            operator storage_ref<T, Fixed, Allocator>();

            auto get_size() const -> size_type;

            auto get_capacity() const -> size_type;

            auto swap_size(storage &other_storage) -> void;

            auto get_is_allocated() const -> bool;

            auto get_allocated_data() -> pointer &;

            auto get_allocated_data() const -> const_pointer;

            auto get_inlined_data() -> pointer;

            auto get_inlined_data() const -> const_pointer;

            auto get_allocated_capacity() const -> size_type;

            auto swap_allocated_capacity(storage &other_storage) -> void;

            auto get_inlined_capacity() const -> size_type;

            auto get_alloc_ptr() -> allocator_type *;

            auto get_alloc_ptr() const -> const allocator_type *;

            auto set_is_allocated() -> void;

            auto unset_is_allocated() -> void;

            auto set_size(size_type size) -> void;

            auto set_allocated_size(size_type size) -> void;

            auto set_inlined_size(size_type size) -> void;

            auto add_size(size_type count) -> void;

            auto subtract_size(size_type count) -> void;

            auto set_allocated_data(pointer ptr, size_type capacity) -> void;

            auto memcpy_from(const storage &other_storage) -> void;

            auto get_storage() -> storage &;

            auto get_storage() const -> const storage &;

        private:
            union data
            {
                allocated<pointer, size_type> allocated_;
                inlined<value_type, N> inlined_;

                data() : allocated_() {}
            };

            data_members<size_type, allocator_type> members_;

            data data_;
        };

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        storage<T, N, Fixed, Allocator>::storage(const allocator_type &alloc) : members_(alloc, 0), data_()
        {
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        storage<T, N, Fixed, Allocator>::storage(size_type n, const allocator_type &alloc) : members_(alloc, 0), data_()
        {
            this->initialize(default_value_adapter(), n, n);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        storage<T, N, Fixed, Allocator>::storage(size_type n, const_reference v, const allocator_type &alloc)
            : members_(alloc, 0), data_()
        {
            this->initialize(copy_value_adapter(v), n, n);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        template <typename InputIt>
        storage<T, N, Fixed, Allocator>::storage(
            InputIt first,
            InputIt last,
            const allocator_type &alloc,
            blessed::enable_if_t<
                blessed::iter::is_input_iterator<InputIt>::value && !blessed::iter::is_forward_iterator<InputIt>::value &&
                std::is_constructible<value_type, typename std::iterator_traits<InputIt>::reference>::value> * /*unused*/)
            : members_(alloc, 0), data_()
        {
            std::copy(first, last, std::back_inserter(*this));
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        template <typename ForwardIt>
        storage<T, N, Fixed, Allocator>::storage(
            ForwardIt first,
            ForwardIt last,
            const allocator_type &alloc,
            blessed::enable_if_t<
                blessed::iter::is_forward_iterator<ForwardIt>::value &&
                std::is_constructible<value_type, typename std::iterator_traits<ForwardIt>::reference>::value> * /*unused*/)
            : members_(alloc, 0), data_()
        {
            auto n = static_cast<size_type>(std::distance(first, last));
            this->initialize(iterator_value_adapter<ForwardIt>(first), n, n);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        storage<T, N, Fixed, Allocator>::storage(const storage &other, const allocator_type &alloc) : members_(alloc, 0)
        {
            if (is_memcpy_ok::value && !other.get_is_allocated())
            {
                this->memcpy_from(other);
            }
            else
            {
                auto data_ptr = other.get_is_allocated() ? other.get_allocated_data() : other.get_inlined_data();
                this->initialize(iterator_value_adapter<const_pointer>(data_ptr), other.get_size(), other.get_capacity());
            }
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        storage<T, N, Fixed, Allocator>::storage(storage &&other) noexcept(std::is_nothrow_move_constructible<value_type>::value)
            : members_(*other.get_alloc_ptr(), 0)
        {
            if (is_memcpy_ok::value)
            {
                memcpy_from(other);

                other.set_inlined_size(0);
            }
            else if (other.get_is_allocated())
            {
                set_allocated_data(other.get_allocated_data(), other.get_allocated_capacity());
                set_allocated_size(other.get_size());

                other.set_inlined_size(0);
            }
            else
            {
                iterator_value_adapter<move_iterator> other_values(move_iterator(other.get_inlined_data()));

                vector_detail::construct_elements(get_alloc_ptr(), get_inlined_data(), &other_values, other.get_size());

                set_inlined_size(other.get_size());
            }
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        storage<T, N, Fixed, Allocator>::storage(storage &&other, const allocator_type &alloc) noexcept(
            std::is_nothrow_move_constructible<value_type>::value)
            : members_(alloc, 0)
        {
            if (is_memcpy_ok::value)
            {
                memcpy_from(other);

                other.set_inlined_size(0);
            }
            else if ((*get_alloc_ptr() == *other.get_alloc_ptr()) && other.get_is_allocated())
            {
                set_allocated_data(other.get_allocated_data(), other.get_allocated_capacity());
                set_allocated_size(other.get_size());

                other.set_inlined_size(0);
            }
            else
            {
                auto data_ptr = other.get_is_allocated() ? other.get_allocated_data() : other.get_inlined_data();
                this->initialize(iterator_value_adapter<move_iterator>(move_iterator(data_ptr)), other.get_size(), other.get_capacity());
            }
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        storage<T, N, Fixed, Allocator>::~storage()
        {
            if (members_.size_ == 0)
            {
                // nothing to do
            }
            else if (is_memcpy_ok::value)
            {
                this->deallocate_if_allocated();
            }
            else
            {
                pointer ptr = get_is_allocated() ? get_allocated_data() : get_inlined_data();
                destroy_elements(get_alloc_ptr(), ptr, get_size());
                this->deallocate_if_allocated();
            }
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        storage<T, N, Fixed, Allocator>::operator storage_ref<T, Fixed, Allocator>()
        {
            return storage_ref<T, Fixed, Allocator>(members_, N, data_.allocated_, get_inlined_data());
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::get_size() const -> size_type
        {
            return members_.size_ >> 1;
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::get_capacity() const -> size_type
        {
            return get_is_allocated() ? get_allocated_capacity() : get_inlined_capacity();
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::swap_size(storage &other_storage) -> void
        {
            using std::swap;
            swap(members_.size_, other_storage.members_.size_);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::get_is_allocated() const -> bool
        {
            return members_.size_ & 1;
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::get_allocated_data() -> pointer &
        {
            return data_.allocated_.allocated_data_;
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::get_allocated_data() const -> const_pointer
        {
            return data_.allocated_.allocated_data_;
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::get_inlined_data() -> pointer
        {
            return reinterpret_cast<pointer>(std::addressof(data_.inlined_));
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::get_inlined_data() const -> const_pointer
        {
            return reinterpret_cast<const_pointer>(std::addressof(data_.inlined_));
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::get_allocated_capacity() const -> size_type
        {
            return data_.allocated_.allocated_capacity_;
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::swap_allocated_capacity(storage &other_storage) -> void
        {
            using std::swap;
            swap(data_.allocated_.allocated_capacity_, other_storage.data_.allocated_.allocated_capacity_);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::get_inlined_capacity() const -> size_type
        {
            return static_cast<size_type>(N);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::get_alloc_ptr() -> allocator_type *
        {
            return std::addressof(members_.allocator_);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::get_alloc_ptr() const -> const allocator_type *
        {
            return std::addressof(members_.allocator_);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::set_is_allocated() -> void
        {
            members_.size_ |= static_cast<size_type>(1);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::unset_is_allocated() -> void
        {
            members_.size_ &= (std::numeric_limits<size_type>::max() - 1);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::set_size(size_type size) -> void
        {
            members_.size_ = (size << 1) | static_cast<size_type>(get_is_allocated());
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::set_allocated_size(size_type size) -> void
        {
            members_.size_ = (size << 1) | static_cast<size_type>(1);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::set_inlined_size(size_type size) -> void
        {
            members_.size_ = size << static_cast<size_type>(1);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::add_size(size_type count) -> void
        {
            members_.size_ += count << static_cast<size_type>(1);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::subtract_size(size_type count) -> void
        {
            assert(count <= get_size());

            members_.size_ -= count << static_cast<size_type>(1);
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::set_allocated_data(pointer ptr, size_type capacity) -> void
        {
            data_.allocated_.allocated_data_ = ptr;
            data_.allocated_.allocated_capacity_ = capacity;
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::memcpy_from(const storage &other_storage) -> void
        {
            assert(is_memcpy_ok::value || other_storage.get_is_allocated());

            members_.size_ = other_storage.members_.size_;
            data_ = other_storage.data_;
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::get_storage() -> storage &
        {
            return *this;
        }

        template <typename T, std::size_t N, bool Fixed, typename Allocator>
        auto storage<T, N, Fixed, Allocator>::get_storage() const -> const storage &
        {
            return *this;
        }

    }
}
