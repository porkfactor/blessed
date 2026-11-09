#ifndef BLESSED_INTRUSIVE_PTR_HPP_
#define BLESSED_INTRUSIVE_PTR_HPP_

#include <functional>
#include <utility>
#include <memory>
#include <atomic>

#include <blessed/blessed.hpp>
#include <blessed/type_traits.hpp>

namespace blessed
{
    namespace detail
    {
        template<typename...>
        using void_t = void;

        template<class T>
        struct identity
        {
            using type = T;
        };

        template<class T, class Void, template<class...> class, class...>
        struct detector : identity<T>
        {
            using value_t = std::false_type;
        };

        template<class T, template<class...> class U, class... Args>
        struct detector<T, void_t<U<Args...>>, U, Args...> : identity<U<Args...>>
        {
            using value_t = std::true_type;
        };

        struct nonesuch final
        {
            nonesuch(nonesuch const &) = delete;
            nonesuch() = delete;
            ~nonesuch() = delete;

            void operator=(nonesuch const &) = delete;
        };

        template<class T, template<class...> class U, class... Args>
        using detected_or = detector<T, void, U, Args...>;

        template<template<class...> class T, class... Args>
        using detected_t = typename detected_or<nonesuch, T, Args...>::type;

        template<class T, template<class...> class U, class... Args>
        using detected_or_t = typename detected_or<T, U, Args...>::type;

        template<template<class...> class T, class... Args>
        using is_detected = typename detected_or<nonesuch, T, Args...>::value_t;

        template<class To, template<class...> class T, class... Args>
        using is_detected_convertible = std::is_convertible<
            detected_t<T, Args...>,
            To>;

        template<class T, template<class...> class U, class... Args>
        using is_detected_exact = std::is_same<T, detected_t<U, Args...>>;

        template<class T>
        using has_pointer = typename T::pointer;

        template<class T>
        using has_default_action = typename T::default_action;

        template<class T, class P>
        using has_use_count = decltype(T::use_count(std::declval<P>()));

    }

    template<class>
    struct retain_traits;

    template<class T>
    struct atomic_reference_count
    {
        template<class>
        friend struct retain_traits;

    protected:
        atomic_reference_count() = default;

    private:
        std::atomic<uint_least64_t> count{1};
    };

    template<typename T>
    struct reference_count
    {
        template<typename>
        friend struct retain_traits;

    protected:
        reference_count() = default;

    private:
        uint_least64_t count{1};
    };

    struct retain_object_t
    {
        retain_object_t() noexcept = default;
    };

    struct adopt_object_t
    {
        adopt_object_t() noexcept = default;
    };

    constexpr retain_object_t retain_object{};
    constexpr adopt_object_t adopt_object{};

    template<class T>
    struct retain_traits final
    {
        template<typename U, typename = ::blessed::enable_if_t<std::is_base_of<U, T>::value>>
        static void increment(atomic_reference_count<U> *ptr) noexcept
        {
            ptr->count.fetch_add(1, std::memory_order_relaxed);
        }

        template<typename U, typename = ::blessed::enable_if_t<std::is_base_of<U, T>::value>>
        static void decrement(atomic_reference_count<U> *ptr) noexcept
        {
            ptr->count.fetch_sub(1, std::memory_order_acq_rel);
            if (!use_count(ptr))
            {
                delete static_cast<T *>(ptr);
            }
        }

        template<typename U, typename = ::blessed::enable_if_t<std::is_base_of<U, T>::value>>
        static long use_count(atomic_reference_count<U> *ptr) noexcept
        {
            return ptr->count.load(std::memory_order_relaxed);
        }

        template<typename U, typename = ::blessed::enable_if_t<std::is_base_of<U, T>::value>>
        static void increment(reference_count<U> *ptr) noexcept
        {
            ++ptr->count;
        }

        template<typename U, typename = ::blessed::enable_if_t<std::is_base_of<U, T>::value>>
        static void decrement(reference_count<U> *ptr) noexcept
        {
            --ptr->count;
            if (!use_count(ptr))
            {
                delete static_cast<T *>(ptr);
            }
        }

        template<typename U, typename = ::blessed::enable_if_t<std::is_base_of<U, T>::value>>
        static long use_count(reference_count<U> *ptr) noexcept
        {
            return ptr->count;
        }
    };

    template<class T, class R = retain_traits<T>>
    struct retain_ptr
    {
        using element_type = T;
        using traits_type = R;

        using pointer = detail::detected_or_t<blessed::add_pointer_t<element_type>, detail::has_pointer, traits_type>;
        using default_action = detail::detected_or_t<adopt_object_t, detail::has_default_action, traits_type>;

        retain_ptr(pointer ptr, retain_object_t) :
            retain_ptr{ptr, adopt_object}
        {
            if (*this)
            {
                traits_type::increment(this->get());
            }
        }

        retain_ptr(pointer ptr, adopt_object_t) noexcept :
            pointer_{ptr}
        {}

        explicit retain_ptr(pointer ptr) :
            retain_ptr{ptr, default_action()}
        {}

        retain_ptr(nullptr_t) :
            retain_ptr{}
        {}

        retain_ptr(retain_ptr const &other) :
            pointer_{other.pointer_}
        {
            if (*this)
            {
                traits_type::increment(this->get());
            }
        }

        retain_ptr(retain_ptr &&other) noexcept :
            pointer_{other.release()}
        {}

        retain_ptr() noexcept = default;
        ~retain_ptr()
        {
            if (*this)
            {
                traits_type::decrement(this->get());
            }
        }

        retain_ptr &operator=(retain_ptr const &other)
        {
            retain_ptr(other).swap(*this);
            return *this;
        }

        retain_ptr &operator=(retain_ptr &&other)
        {
            retain_ptr(std::move(other)).swap(*this);
            return *this;
        }

        retain_ptr &operator=(nullptr_t) noexcept
        {
            this->reset();
            return *this;
        }

        void swap(retain_ptr &other) noexcept
        {
            using std::swap;
            swap(this->pointer_, other.pointer_);
        }

        explicit operator pointer() const noexcept
        {
            return this->get();
        }

        explicit operator bool() const noexcept
        {
            return this->get();
        }

        element_type &operator*() const noexcept
        {
            return *this->get();
        }

        pointer operator->() const noexcept
        {
            return this->get();
        }

        pointer get() const noexcept
        {
            return pointer_;
        }

        long use_count() const
        {
            if (detail::has_use_count<traits_type, element_type>())
            {
                return this->get() ? traits_type::use_count(this->get()) : 0;
            }
            else
            {
                return -1;
            }
        }

        pointer release() noexcept
        {
            auto ptr = this->get();
            this->pointer_ = pointer{};
            return ptr;
        }

        void reset(pointer ptr, retain_object_t)
        {
            *this = retain_ptr(ptr, retain_object);
        }

        void reset(pointer ptr, adopt_object_t) noexcept
        {
            *this = retain_ptr(ptr, adopt_object);
        }

        void reset(pointer ptr = pointer{})
        {
            *this = retain_ptr(ptr, default_action());
        }

    private:
        pointer pointer_{};
    };

    template<class T, class R>
    void swap(retain_ptr<T, R> &lhs, retain_ptr<T, R> &rhs) noexcept
    {
        lhs.swap(rhs);
    }

    template<class T, class R>
    bool operator==(retain_ptr<T, R> const &lhs, retain_ptr<T, R> const &rhs) noexcept
    {
        return lhs.get() == rhs.get();
    }

    template<class T, class R>
    bool operator!=(retain_ptr<T, R> const &lhs, retain_ptr<T, R> const &rhs) noexcept
    {
        return lhs.get() != rhs.get();
    }

    template<class T, class R>
    bool operator>=(retain_ptr<T, R> const &lhs, retain_ptr<T, R> const &rhs) noexcept
    {
        return lhs.get() >= rhs.get();
    }

    template<class T, class R>
    bool operator<=(retain_ptr<T, R> const &lhs, retain_ptr<T, R> const &rhs) noexcept
    {
        return lhs.get() <= rhs.get();
    }

    template<class T, class R>
    bool operator>(retain_ptr<T, R> const &lhs, retain_ptr<T, R> const &rhs) noexcept
    {
        return lhs.get() > rhs.get();
    }

    template<class T, class R>
    bool operator<(retain_ptr<T, R> const &lhs, retain_ptr<T, R> const &rhs) noexcept
    {
        return lhs.get() < rhs.get();
    }

    template<class T, class R>
    bool operator==(retain_ptr<T, R> const &lhs, nullptr_t) noexcept
    {
        return bool(lhs);
    }

    template<class T, class R>
    bool operator!=(retain_ptr<T, R> const &lhs, nullptr_t) noexcept
    {
        return !lhs;
    }

    template<class T, class R>
    bool operator>=(retain_ptr<T, R> const &lhs, nullptr_t) noexcept
    {
        return !(lhs < nullptr);
    }

    template<class T, class R>
    bool operator<=(retain_ptr<T, R> const &lhs, nullptr_t) noexcept
    {
        return nullptr < lhs;
    }

    template<class T, class R>
    bool operator>(retain_ptr<T, R> const &lhs, nullptr_t) noexcept
    {
        return !(nullptr < lhs);
    }

    template<class T, class R>
    bool operator<(retain_ptr<T, R> const &lhs, nullptr_t) noexcept
    {
        return std::less<typename retain_ptr<T, R>::pointer>()(lhs.get(), nullptr);
    }

    template<class T, class R>
    bool operator==(nullptr_t, retain_ptr<T, R> const &rhs) noexcept
    {
        return !rhs;
    }

    template<class T, class R>
    bool operator!=(nullptr_t, retain_ptr<T, R> const &rhs) noexcept
    {
        return bool(rhs);
    }

    template<class T, class R>
    bool operator>=(nullptr_t, retain_ptr<T, R> const &rhs) noexcept
    {
        return !(nullptr < rhs);
    }

    template<class T, class R>
    bool operator<=(nullptr_t, retain_ptr<T, R> const &rhs) noexcept
    {

    }

    template<class T, class R>
    bool operator>(nullptr_t, retain_ptr<T, R> const &rhs) noexcept
    {
        return !(rhs < nullptr);
    }

    template<class T, class R>
    bool operator<(nullptr_t, retain_ptr<T, R> const &rhs) noexcept
    {
        return std::less<typename retain_ptr<T, R>::pointer>()(rhs.get(), nullptr);
    }
}

#endif

