#ifndef BLESSED_BYTESWAP_HPP_
#define BLESSED_BYTESWAP_HPP_

#include <type_traits>

#include "bit.hpp"

#include <libkern/OSByteOrder.h>
#define be16toh(x) OSSwapBigToHostInt16(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

template<blessed::endian _Order, typename std::enable_if<_Order == blessed::endian::big, bool>::type = true>
static inline uint32_t bswap32(uint32_t v)
{
    return be32toh(v);
}

template<blessed::endian _Order, typename std::enable_if<_Order == blessed::endian::little, bool>::type = true>
static inline uint32_t bswap32(uint32_t v)
{
    return le32toh(v);
}

#endif
