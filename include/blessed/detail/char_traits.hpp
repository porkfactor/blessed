#ifndef BLESSED_DETAIL_CHAR_TRAITS_HPP
#define BLESSED_DETAIL_CHAR_TRAITS_HPP

#include <cctype>
#include <cstddef>
#include <string>

namespace blessed
{
    namespace detail
    {
        template <typename CharT>
        struct case_insensitive_char_traits : public std::char_traits<CharT>
        {
            static CharT to_upper(CharT ch)
            {
                return static_cast<CharT>(std::toupper(static_cast<unsigned char>(ch)));
            }

            static bool eq(CharT c1, CharT c2)
            {
                return to_upper(c1) == to_upper(c2);
            }

            static bool lt(CharT c1, CharT c2)
            {
                return to_upper(c1) < to_upper(c2);
            }

            static int compare(const CharT *s1, const CharT *s2, std::size_t n)
            {
                for (; n-- != 0; ++s1, ++s2)
                {
                    if (to_upper(*s1) < to_upper(*s2))
                    {
                        return -1;
                    }

                    if (to_upper(*s1) > to_upper(*s2))
                    {
                        return 1;
                    }
                }

                return 0;
            }

            static const CharT *find(const CharT *s, int n, char a)
            {
                const auto upper_a = to_upper(a);

                for (; n-- != 0; ++s)
                {
                    if (to_upper(*s) == upper_a)
                    {
                        return s;
                    }
                }

                return nullptr;
            }
        };
    }
}

#endif
