#ifndef BLESSED_MEMORY_HPP_
#define BLESSED_MEMORY_HPP_

#include <memory>

namespace blessed
{
    namespace detail
    {
        struct c_deleter
        {
            void operator()(void *p) const { ::free(p); }
        };
    }

    template<typename _Type>
    using unique_c_ptr = std::unique_ptr<_Type, detail::c_deleter>;

    template<typename _Type>
    using shared_c_ptr = std::shared_ptr<_Type>;

    template<typename _Type = void>
    unique_c_ptr<_Type> malloc_unique(std::size_t n)
    {
        return unique_c_ptr<_Type>(static_cast<_Type *>(::calloc(n, sizeof(_Type))));
    }

    template<typename _Type = void>
    shared_c_ptr<_Type> malloc_shared(std::size_t n)
    {
        return shared_c_ptr<_Type>(static_cast<_Type *>(::calloc(n, sizeof(_Type))), detail::c_deleter{});
    }
}

#endif
