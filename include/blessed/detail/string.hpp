#ifndef BLESSED_DETAIL_STRING_HPP
#define BLESSED_DETAIL_STRING_HPP

#include <blessed/blessed.hpp>
#include <blessed/hash.hpp>

#include <cstddef>
#include <cstring>
#include <iterator>

namespace blessed
{
    namespace algo
    {
        namespace detail
        {
            template <typename CharT, typename SizeType, typename Traits, SizeType npos>
            inline SizeType BLESSED_CXX14_CONSTEXPR str_find(const CharT *p, SizeType size, CharT c, SizeType pos) noexcept
            {
                if (pos >= size)
                {
                    return npos;
                }

                const CharT *r = Traits::find(p + pos, size - pos, c);

                if (r == nullptr)
                {
                    return npos;
                }

                return static_cast<SizeType>(r - p);
            }

            template <typename CharT, typename Traits>
            inline BLESSED_CXX14_CONSTEXPR const CharT *
            search_substring(const CharT *first1, const CharT *last1, const CharT *first2, const CharT *last2)
            {
                // Take advantage of knowing source and pattern lengths.
                // Stop short when source is smaller than pattern.
                const ptrdiff_t len2 = last2 - first2;
                if (len2 == 0)
                {
                    return first1;
                }

                ptrdiff_t len1 = last1 - first1;
                if (len1 < len2)
                {
                    return last1;
                }

                // First element of __first2 is loop invariant.
                CharT f2 = *first2;
                while (true)
                {
                    len1 = last1 - first1;
                    // Check whether __first1 still has at least __len2 bytes.
                    if (len1 < len2)
                    {
                        return last1;
                    }

                    // Find __f2 the first byte matching in __first1.
                    first1 = Traits::find(first1, static_cast<std::size_t>(len1 - len2 + 1), f2);
                    if (first1 == nullptr)
                    {
                        return last1;
                    }

                    // It is faster to compare from the first byte of first1 even if we
                    // already know that it matches the first byte of first2: this is because
                    // first2 is most likely aligned, as it is user's "pattern" string, and
                    // first1 + 1 is most likely not aligned, as the match is in the middle of
                    // the string.
                    if (Traits::compare(first1, first2, static_cast<std::size_t>(len2)) == 0)
                    {
                        return first1;
                    }

                    ++first1;
                }
            }

            template <typename BinaryPredicate, typename RandomAccessIterator1, typename RandomAccessIterator2>
            inline RandomAccessIterator1 find_end(
                RandomAccessIterator1 first1,
                RandomAccessIterator1 last1,
                RandomAccessIterator2 first2,
                RandomAccessIterator2 last2,
                BinaryPredicate pred,
                std::random_access_iterator_tag /* unused */,
                std::random_access_iterator_tag /* unused */)
            {
                typename std::iterator_traits<RandomAccessIterator2>::difference_type len2 = last2 - first2;
                if (len2 == 0)
                {
                    return last1;
                }

                typename std::iterator_traits<RandomAccessIterator1>::difference_type len1 = last1 - first1;
                if (len1 < len2)
                {
                    return last1;
                }

                const RandomAccessIterator1 s = first1 + (len2 - 1); // End of pattern match can't go before here
                RandomAccessIterator1 l1 = last1;
                RandomAccessIterator1 l2 = last2;
                --l2;

                while (true)
                {
                    while (true)
                    {
                        if (s == l1)
                        {
                            return last1;
                        }

                        if (pred(*--l1, *l2))
                        {
                            break;
                        }
                    }

                    RandomAccessIterator1 m1 = l1;
                    RandomAccessIterator2 m2 = l2;

                    while (true)
                    {
                        if (m2 == first2)
                        {
                            return m1;
                        }

                        // no need to check range on __m1 because __s guarantees we have enough source
                        if (!pred(*--m1, *--m2))
                        {
                            break;
                        }
                    }
                }
            }

            template <typename CharT, typename SizeType, typename Traits, SizeType npos>
            inline SizeType BLESSED_CXX14_CONSTEXPR
            str_find(const CharT *p, SizeType size, const CharT *s, SizeType pos, SizeType n) noexcept
            {
                if (pos > size)
                {
                    return npos;
                }

                if (n == 0)
                {
                    return pos;
                }

                const CharT *r = search_substring<CharT, Traits>(p + pos, p + size, s, s + n);

                if (r == p + size)
                {
                    return npos;
                }

                return static_cast<SizeType>(r - p);
            }

            template <typename CharT, typename SizeType, typename Traits, SizeType npos>
            inline SizeType BLESSED_CXX14_CONSTEXPR str_rfind(const CharT *p, SizeType size, CharT c, SizeType pos) noexcept
            {
                if (size < 1)
                {
                    return npos;
                }

                if (pos < size)
                {
                    ++pos;
                }
                else
                {
                    pos = size;
                }

                for (const CharT *ps = p + pos; ps != p;)
                {
                    if (Traits::eq(*--ps, c))
                    {
                        return static_cast<SizeType>(ps - p);
                    }
                }

                return npos;
            }

            template <typename CharT, typename SizeType, typename Traits, SizeType npos>
            inline SizeType BLESSED_CXX14_CONSTEXPR
            str_rfind(const CharT *p, SizeType size, const CharT *s, SizeType pos, SizeType n) noexcept
            {
                pos = std::min(pos, size);
                if (n < size - pos)
                {
                    pos += n;
                }
                else
                {
                    pos = size;
                }

                const CharT *r =
                    find_end(p, p + pos, s, s + n, Traits::eq, std::random_access_iterator_tag(), std::random_access_iterator_tag());

                if (n > 0 && r == p + pos)
                {
                    return npos;
                }

                return static_cast<SizeType>(r - p);
            }

            template <typename ForwardIterator1, typename ForwardIterator2, typename BinaryPedicate>
            inline ForwardIterator1 BLESSED_CXX14_CONSTEXPR find_first_of_ce(
                ForwardIterator1 first1,
                ForwardIterator1 last1,
                ForwardIterator2 first2,
                ForwardIterator2 last2,
                BinaryPedicate pred)
            {
                for (; first1 != last1; ++first1)
                {
                    for (ForwardIterator2 j = first2; j != last2; ++j)
                    {
                        if (pred(*first1, *j))
                        {
                            return first1;
                        }
                    }
                }

                return last1;
            }

            template <typename CharT, typename SizeType, typename Traits, SizeType npos>
            inline SizeType BLESSED_CXX14_CONSTEXPR
            str_find_first_of(const CharT *p, SizeType size, const CharT *s, SizeType pos, SizeType n) noexcept
            {
                if (pos >= size || n == 0)
                {
                    return npos;
                }

                const CharT *r = find_first_of_ce(p + pos, p + size, s, s + n, Traits::eq);

                if (r == p + size)
                {
                    return npos;
                }

                return static_cast<SizeType>(r - p);
            }

            template <typename CharT, typename SizeType, typename Traits, SizeType npos>
            inline SizeType BLESSED_CXX14_CONSTEXPR
            str_find_last_of(const CharT *p, SizeType size, const CharT *s, SizeType pos, SizeType n) noexcept
            {
                if (n != 0)
                {
                    if (pos < size)
                    {
                        ++pos;
                    }
                    else
                    {
                        pos = size;
                    }

                    for (const CharT *ps = p + pos; ps != p;)
                    {
                        const CharT *r = Traits::find(s, n, *--ps);
                        if (r)
                        {
                            return static_cast<SizeType>(ps - p);
                        }
                    }
                }

                return npos;
            }

            template <typename CharT, typename SizeType, typename Traits, SizeType npos>
            inline SizeType BLESSED_CXX14_CONSTEXPR
            str_find_first_not_of(const CharT *p, SizeType size, const CharT *s, SizeType pos, SizeType n) noexcept
            {
                if (pos < size)
                {
                    const CharT *pe = p + size;
                    for (const CharT *ps = p + pos; ps != pe; ++ps)
                    {
                        if (Traits::find(s, n, *ps) == nullptr)
                        {
                            return static_cast<SizeType>(ps - p);
                        }
                    }
                }

                return npos;
            }

            template <typename CharT, typename SizeType, typename Traits, SizeType npos>
            inline SizeType BLESSED_CXX14_CONSTEXPR str_find_first_not_of(const CharT *p, SizeType size, CharT c, SizeType pos) noexcept
            {
                if (pos < size)
                {
                    const CharT *pe = p + size;
                    for (const CharT *ps = p + pos; ps != pe; ++ps)
                    {
                        if (!Traits::eq(*ps, c))
                        {
                            return static_cast<SizeType>(ps - p);
                        }
                    }
                }

                return npos;
            }

            template <typename CharT, typename SizeType, typename Traits, SizeType npos>
            inline SizeType BLESSED_CXX14_CONSTEXPR
            str_find_last_not_of(const CharT *p, SizeType size, const CharT *s, SizeType pos, SizeType n) noexcept
            {
                if (pos < size)
                {
                    ++pos;
                }
                else
                {
                    pos = size;
                }

                for (const CharT *ps = p + pos; ps != p;)
                {
                    if (Traits::find(s, n, *--ps) == nullptr)
                    {
                        return static_cast<SizeType>(ps - p);
                    }
                }

                return npos;
            }

            template <typename CharT, typename SizeType, typename Traits, SizeType npos>
            inline SizeType BLESSED_CXX14_CONSTEXPR str_find_last_not_of(const CharT *p, SizeType size, CharT c, SizeType pos) noexcept
            {
                if (pos < size)
                {
                    ++pos;
                }
                else
                {
                    pos = size;
                }

                for (const CharT *ps = p + pos; ps != p;)
                {
                    if (!Traits::eq(*--ps, c))
                    {
                        return static_cast<SizeType>(ps - p);
                    }
                }

                return npos;
            }
        }
    }
}

#endif
