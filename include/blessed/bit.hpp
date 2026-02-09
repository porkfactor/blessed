#ifndef BLESSED_BIT_HPP_
#define BLESSED_BIT_HPP_

namespace blessed
{
    enum class endian
    {
        little = __ORDER_LITTLE_ENDIAN__,
        big = __ORDER_BIG_ENDIAN__,
        native = __BYTE_ORDER__
    };
}

#endif
