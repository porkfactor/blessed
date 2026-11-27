#ifndef BLESSED_UTILITY_ALIGN_HPP
#define BLESSED_UTILITY_ALIGN_HPP

#include <array>

namespace blessed
{
    namespace detail
    {
        template <typename T, typename... Ts>
        class align_impl
        {
            T t_;
            align_impl<Ts...> rest_;

        public:
            align_impl() = delete;
        };

        template <typename T>
        class align_impl<T>
        {
            T t_;

        public:
            align_impl() = delete;
        };

        template <typename T, typename... Ts>
        union size_impl
        {
            char arr[sizeof(T)];

            size_impl<Ts...> rest;
        };

        template <typename T>
        union size_impl<T>
        {
            char arr[sizeof(T)];
        };

    }

    template <typename T, typename... Ts>
    struct aligned_storage
    {
        alignas(detail::align_impl<T, Ts...>) char buffer[sizeof(detail::size_impl<T, Ts...>)];
    };
}

#endif
