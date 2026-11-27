#ifndef BLESSED_ITERATOR_HPP
#define BLESSED_ITERATOR_HPP

#include <blessed/blessed.hpp>
#include <blessed/type_traits.hpp>

#include <initializer_list>
#include <iterator>
#include <type_traits>

namespace blessed
{
    namespace iter
    {
        namespace detail
        {
            template <typename T>
            struct has_iterator_category
            {
            private:
                struct two
                {
                    char lx;
                    char lxx;
                };

                template <typename U>
                static two test(...);

                template <typename U>
                static char test(typename U::iterator_category * = nullptr);

            public:
                static const bool value = sizeof(test<T>(nullptr)) == 1;
            };

            template <typename T, typename U, bool = detail::has_iterator_category<std::iterator_traits<T>>::value>
            struct has_iterator_category_convertible_to
                : public std::integral_constant<bool, std::is_convertible<typename std::iterator_traits<T>::iterator_category, U>::value>
            {
            };

            template <typename T, typename U>
            struct has_iterator_category_convertible_to<T, U, false> : public std::false_type
            {
            };
        }

        template <typename T>
        struct is_input_iterator : public detail::has_iterator_category_convertible_to<T, std::input_iterator_tag>
        {
        };

        template <typename T>
        struct is_forward_iterator : public detail::has_iterator_category_convertible_to<T, std::forward_iterator_tag>
        {
        };

        template <typename T>
        struct is_bidirectional_iterator : public detail::has_iterator_category_convertible_to<T, std::bidirectional_iterator_tag>
        {
        };

        template <typename T>
        struct is_random_access_iterator : public detail::has_iterator_category_convertible_to<T, std::random_access_iterator_tag>
        {
        };
    }
}

#endif
