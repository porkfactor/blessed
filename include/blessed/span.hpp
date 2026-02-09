#ifndef BLESSED_SPAN_HPP_
#define BLESSED_SPAN_HPP_

#include <blessed/blessed.hpp>
#include <blessed/cstddef.hpp>
#include <blessed/type_traits.hpp>

#if BLESSED_CXX23 || (defined(__cpp_lib_span) && __cpp_lib_span >= 202002)
#include <span>

namespace blessed
{
    template <typename _Type, std::size_t _Extent = std::dynamic_extent>
    using span = std::span<_Type, _Extent>;
}
#else
#include <limits>
#include <array>

namespace blessed
{
    static constexpr std::size_t dynamic_extent = std::numeric_limits<std::size_t>::max();

    template<typename, std::size_t = dynamic_extent>
    class span;

    // implementation detail
    namespace detail
    {
        template <class T>
        struct is_span_oracle : std::false_type
        {
        };

        template <class ElementType, std::size_t Extent>
        struct is_span_oracle<blessed::span<ElementType, Extent>> : std::true_type
        {
        };

        template <class T>
        struct is_span : public is_span_oracle<blessed::remove_cv_t<T>>
        {
        };

        template <class T>
        struct is_std_array_oracle : std::false_type
        {
        };

        template <class ElementType, std::size_t Extent>
        struct is_std_array_oracle<std::array<ElementType, Extent>> : std::true_type
        {
        };

        template <class T>
        struct is_std_array : is_std_array_oracle<blessed::remove_cv_t<T>>
        {
        };

        template <std::size_t From, std::size_t To>
        struct is_allowed_extent_conversion
            : std::integral_constant<bool, From == To || To == dynamic_extent>
        {
        };

        template <class From, class To>
        struct is_allowed_element_type_conversion
            : std::integral_constant<bool, std::is_convertible<From (*)[], To (*)[]>::value>
        {
        };

        template <class Type>
        class span_iterator
        {
        public:
#if defined(__cpp_lib_ranges) || (defined(_MSVC_STL_VERSION) && defined(__cpp_lib_concepts))
            using iterator_concept = std::contiguous_iterator_tag;
#endif // __cpp_lib_ranges
            using iterator_category = std::random_access_iterator_tag;
            using value_type = blessed::remove_cv_t<Type>;
            using difference_type = std::ptrdiff_t;
            using pointer = Type *;
            using reference = Type &;

#ifdef _MSC_VER
            using _Unchecked_type = pointer;
#endif // _MSC_VER
            BLESSED_CXX14_CONSTEXPR span_iterator() = default;

            BLESSED_CXX14_CONSTEXPR span_iterator(pointer begin, pointer end, pointer current)
                : begin_(begin), end_(end), current_(current)
            {
            }

            BLESSED_CXX14_CONSTEXPR operator span_iterator<const Type>() const noexcept
            {
                return {begin_, end_, current_};
            }

            BLESSED_CXX14_CONSTEXPR reference operator*() const noexcept
            {
                assert(begin_ && end_);
                assert(begin_ <= current_ && current_ < end_);
                return *current_;
            }

            BLESSED_CXX14_CONSTEXPR pointer operator->() const noexcept
            {
                assert(begin_ && end_);
                assert(begin_ <= current_ && current_ < end_);
                return current_;
            }

            BLESSED_CXX14_CONSTEXPR span_iterator &operator++() noexcept
            {
                assert(begin_ && current_ && end_);
                assert(current_ < end_);

                ++current_;
                return *this;
            }

            BLESSED_CXX14_CONSTEXPR span_iterator operator++(int) noexcept
            {
                span_iterator ret = *this;
                ++*this;
                return ret;
            }

            BLESSED_CXX14_CONSTEXPR span_iterator &operator--() noexcept
            {
                assert(begin_ && end_);
                assert(begin_ < current_);
                --current_;
                return *this;
            }

            BLESSED_CXX14_CONSTEXPR span_iterator operator--(int) noexcept
            {
                span_iterator ret = *this;
                --*this;
                return ret;
            }

            BLESSED_CXX14_CONSTEXPR span_iterator &operator+=(const difference_type n) noexcept
            {
                if (n != 0)
                    assert(begin_ && current_ && end_);
                if (n > 0)
                    assert(end_ - current_ >= n);
                if (n < 0)
                    assert(current_ - begin_ >= -n);

                current_ += n;
                return *this;
            }

            BLESSED_CXX14_CONSTEXPR span_iterator operator+(const difference_type n) const noexcept
            {
                span_iterator ret = *this;
                ret += n;
                return ret;
            }

            friend BLESSED_CXX14_CONSTEXPR span_iterator operator+(const difference_type n, const span_iterator &rhs) noexcept
            {
                return rhs + n;
            }

            BLESSED_CXX14_CONSTEXPR span_iterator &operator-=(const difference_type n) noexcept
            {
                if (n != 0)
                    assert(begin_ && current_ && end_);
                if (n > 0)
                    assert(current_ - begin_ >= n);
                if (n < 0)
                    assert(end_ - current_ >= -n);
                current_ -= n;
                return *this;
            }

            BLESSED_CXX14_CONSTEXPR span_iterator operator-(const difference_type n) const noexcept
            {
                span_iterator ret = *this;
                ret -= n;
                return ret;
            }

            template <
                class Type2,
                blessed::enable_if_t<std::is_same<blessed::remove_cv_t<Type2>, value_type>::value, int> = 0>
            BLESSED_CXX14_CONSTEXPR difference_type operator-(const span_iterator<Type2> &rhs) const noexcept
            {
                assert(begin_ == rhs.begin_ && end_ == rhs.end_);
                return current_ - rhs.current_;
            }

            BLESSED_CXX14_CONSTEXPR reference operator[](const difference_type n) const noexcept
            {
                return *(*this + n);
            }

            template <class Type2, blessed::enable_if_t<std::is_same<blessed::remove_cv_t<Type2>, value_type>::value, int> = 0>
            BLESSED_CXX14_CONSTEXPR bool operator==(const span_iterator<Type2> &rhs) const noexcept
            {
                assert(begin_ == rhs.begin_ && end_ == rhs.end_);
                return current_ == rhs.current_;
            }

            template <class Type2, blessed::enable_if_t<std::is_same<blessed::remove_cv_t<Type2>, value_type>::value, int> = 0>
            BLESSED_CXX14_CONSTEXPR bool operator!=(const span_iterator<Type2> &rhs) const noexcept
            {
                return !(*this == rhs);
            }

            template <class Type2, blessed::enable_if_t<std::is_same<blessed::remove_cv_t<Type2>, value_type>::value, int> = 0>
            BLESSED_CXX14_CONSTEXPR bool operator<(const span_iterator<Type2> &rhs) const noexcept
            {
                assert(begin_ == rhs.begin_ && end_ == rhs.end_);
                return current_ < rhs.current_;
            }

            template <class Type2, blessed::enable_if_t<std::is_same<blessed::remove_cv_t<Type2>, value_type>::value, int> = 0>
            BLESSED_CXX14_CONSTEXPR bool operator>(const span_iterator<Type2> &rhs) const noexcept
            {
                return rhs < *this;
            }

            template <class Type2, blessed::enable_if_t<std::is_same<blessed::remove_cv_t<Type2>, value_type>::value, int> = 0>
            BLESSED_CXX14_CONSTEXPR bool operator<=(const span_iterator<Type2> &rhs) const noexcept
            {
                return !(rhs < *this);
            }

            template <class Type2, blessed::enable_if_t<std::is_same<blessed::remove_cv_t<Type2>, value_type>::value, int> = 0>
            BLESSED_CXX14_CONSTEXPR bool operator>=(const span_iterator<Type2> &rhs) const noexcept
            {
                return !(*this < rhs);
            }

#ifdef _MSC_VER
            // MSVC++ iterator debugging support; allows STL algorithms in 15.8+
            // to unwrap span_iterator to a pointer type after a range check in STL
            // algorithm calls
            friend constexpr void _Verify_range(span_iterator lhs, span_iterator rhs) noexcept
            {                                   // test that [lhs, rhs) forms a valid range inside an STL algorithm
                assert(lhs.begin_ == rhs.begin_ // range spans have to match
                       && lhs.end_ == rhs.end_ &&
                       lhs.current_ <= rhs.current_); // range must not be transposed
            }

            constexpr void _Verify_offset(const difference_type n) const noexcept
            { // test that *this + n is within the range of this call
                if (n != 0)
                    assert(begin_ && current_ && end_);
                if (n > 0)
                    assert(end_ - current_ >= n);
                if (n < 0)
                    assert(current_ - begin_ >= -n);
            }

            constexpr pointer _Unwrapped() const noexcept
            { // after seeking *this to a high water mark, or using one of the
                // _Verify_xxx functions above, unwrap this span_iterator to a raw
                // pointer
                return current_;
            }

            // Tell the STL that span_iterator should not be unwrapped if it can't
            // validate in advance, even in release / optimized builds:
#if defined(GSL_USE_STATIC_CONSTEXPR_WORKAROUND)
            static constexpr const bool _Unwrap_when_unverified = false;
#else
            static constexpr bool _Unwrap_when_unverified = false;
#endif

            constexpr void _Seek_to(const pointer p) noexcept
            { // adjust the position of *this to previously verified location p
                // after _Unwrapped
                current_ = p;
            }
#endif

            pointer begin_ = nullptr;
            pointer end_ = nullptr;
            pointer current_ = nullptr;

            template <typename Ptr>
            friend struct std::pointer_traits;
        };
    }
}

namespace std
{
    template <class Type>
    struct pointer_traits<::blessed::detail::span_iterator<Type>>
    {
        using pointer = ::blessed::detail::span_iterator<Type>;
        using element_type = Type;
        using difference_type = ptrdiff_t;

        static constexpr element_type *to_address(const pointer i) noexcept { return i.current_; }
    };
}

namespace blessed
{
    namespace detail
    {
        template <std::size_t Ext>
        class extent_type
        {
        public:
            using size_type = std::size_t;

            BLESSED_CXX14_CONSTEXPR extent_type() noexcept = default;

            BLESSED_CXX14_CONSTEXPR explicit extent_type(extent_type<dynamic_extent>);

            BLESSED_CXX14_CONSTEXPR explicit extent_type(size_type size) { assert(size == Ext); }

            BLESSED_CXX14_CONSTEXPR size_type size() const noexcept { return Ext; }

        private:
#if defined(GSL_USE_STATIC_CONSTEXPR_WORKAROUND)
            static constexpr const size_type size_ = Ext; // static size equal to Ext
#else
            static constexpr size_type size_ = Ext; // static size equal to Ext
#endif
        };

        template <>
        class extent_type<dynamic_extent>
        {
        public:
            using size_type = std::size_t;

            template <size_type Other>
            BLESSED_CXX14_CONSTEXPR explicit extent_type(extent_type<Other> ext) : size_(ext.size())
            {
            }

            BLESSED_CXX14_CONSTEXPR explicit extent_type(size_type size) : size_(size)
            {
                assert(size != dynamic_extent);
            }

            BLESSED_CXX14_CONSTEXPR size_type size() const noexcept { return size_; }

        private:
            size_type size_;
        };

        template <std::size_t Ext>
        BLESSED_CXX14_CONSTEXPR extent_type<Ext>::extent_type(extent_type<dynamic_extent> ext)
        {
            assert(ext.size() == Ext);
        }

        template <class ElementType, std::size_t Extent, std::size_t Offset, std::size_t Count>
        struct calculate_subspan_type
        {
            using type = span<ElementType, Count != dynamic_extent
                                               ? Count
                                               : (Extent != dynamic_extent ? Extent - Offset : Extent)>;
        };
    } // namespace detail

    // [span], class template span
    template <class ElementType, std::size_t Extent>
    class span
    {
    public:
        // constants and types
        using element_type = ElementType;
        using value_type = blessed::remove_cv_t<ElementType>;
        using size_type = std::size_t;
        using pointer = element_type *;
        using const_pointer = const element_type *;
        using reference = element_type &;
        using const_reference = const element_type &;
        using difference_type = std::ptrdiff_t;

        using iterator = detail::span_iterator<ElementType>;
        using reverse_iterator = std::reverse_iterator<iterator>;

#if defined(GSL_USE_STATIC_CONSTEXPR_WORKAROUND)
        static constexpr const size_type extent{Extent};
#else
        static constexpr size_type extent{Extent};
#endif

        // [span.cons], span constructors, copy, assignment, and destructor
        template <bool Dependent = false,
                  // "Dependent" is needed to make "blessed::enable_if_t<Dependent || Extent == 0 || Extent
                  // == dynamic_extent>" SFINAE, since "blessed::enable_if_t<Extent == 0 || Extent ==
                  // dynamic_extent>" is ill-formed when Extent is greater than 0.
                  class = blessed::enable_if_t<(Dependent ||
                                                detail::is_allowed_extent_conversion<0, Extent>::value)>>
        BLESSED_CXX14_CONSTEXPR span() noexcept : storage_(nullptr, detail::extent_type<0>())
        {
        }

        template <std::size_t MyExtent = Extent, blessed::enable_if_t<MyExtent != dynamic_extent, int> = 0>
        BLESSED_CXX14_CONSTEXPR explicit span(pointer ptr, size_type count) noexcept : storage_(ptr, count)
        {
            assert(count == Extent);
        }

        template <std::size_t MyExtent = Extent, blessed::enable_if_t<MyExtent == dynamic_extent, int> = 0>
        BLESSED_CXX14_CONSTEXPR span(pointer ptr, size_type count) noexcept : storage_(ptr, count)
        {
        }

        template <std::size_t MyExtent = Extent, blessed::enable_if_t<MyExtent != dynamic_extent, int> = 0>
        BLESSED_CXX14_CONSTEXPR explicit span(pointer firstElem, pointer lastElem) noexcept
            : storage_(firstElem, narrow_cast<std::size_t>(lastElem - firstElem))
        {
            assert(lastElem - firstElem == static_cast<difference_type>(Extent));
        }

        template <std::size_t MyExtent = Extent, blessed::enable_if_t<MyExtent == dynamic_extent, int> = 0>
        BLESSED_CXX14_CONSTEXPR span(pointer firstElem, pointer lastElem) noexcept
            : storage_(firstElem, narrow_cast<std::size_t>(lastElem - firstElem))
        {
        }

        template <std::size_t N,
                  blessed::enable_if_t<detail::is_allowed_extent_conversion<N, Extent>::value, int> = 0>
        BLESSED_CXX14_CONSTEXPR span(element_type (&arr)[N]) noexcept
            : storage_(KnownNotNull{arr}, detail::extent_type<N>())
        {
        }

        template <
            class T, std::size_t N,
            blessed::enable_if_t<(detail::is_allowed_extent_conversion<N, Extent>::value &&
                                  detail::is_allowed_element_type_conversion<T, element_type>::value),
                                 int> = 0>
        BLESSED_CXX14_CONSTEXPR span(std::array<T, N> &arr) noexcept
            : storage_(KnownNotNull{arr.data()}, detail::extent_type<N>())
        {
        }

        template <class T, std::size_t N,
                  blessed::enable_if_t<
                      (detail::is_allowed_extent_conversion<N, Extent>::value &&
                       detail::is_allowed_element_type_conversion<const T, element_type>::value),
                      int> = 0>
        constexpr span(const std::array<T, N> &arr) noexcept
            : storage_(KnownNotNull{arr.data()}, detail::extent_type<N>())
        {
        }

        // NB: the SFINAE on these constructors uses .data() as an incomplete/imperfect proxy for the
        // requirement on Container to be a contiguous sequence container.
        template <std::size_t MyExtent = Extent, class Container,
                  blessed::enable_if_t<
                      MyExtent != dynamic_extent && !detail::is_span<Container>::value &&
                          !detail::is_std_array<Container>::value &&
                          std::is_pointer<decltype(std::declval<Container &>().data())>::value &&
                          std::is_convertible<
                              blessed::remove_pointer_t<decltype(std::declval<Container &>().data())> (*)[],
                              element_type (*)[]>::value,
                      int> = 0>
        constexpr explicit span(Container &cont) noexcept : span(cont.data(), cont.size())
        {
        }

        template <std::size_t MyExtent = Extent, class Container,
                  blessed::enable_if_t<
                      MyExtent == dynamic_extent && !detail::is_span<Container>::value &&
                          !detail::is_std_array<Container>::value &&
                          std::is_pointer<decltype(std::declval<Container &>().data())>::value &&
                          std::is_convertible<
                              blessed::remove_pointer_t<decltype(std::declval<Container &>().data())> (*)[],
                              element_type (*)[]>::value,
                      int> = 0>
        constexpr span(Container &cont) noexcept : span(cont.data(), cont.size())
        {
        }

        template <
            std::size_t MyExtent = Extent, class Container,
            blessed::enable_if_t<
                MyExtent != dynamic_extent && std::is_const<element_type>::value &&
                    !detail::is_span<Container>::value && !detail::is_std_array<Container>::value &&
                    std::is_pointer<decltype(std::declval<const Container &>().data())>::value &&
                    std::is_convertible<
                        blessed::remove_pointer_t<decltype(std::declval<const Container &>().data())> (*)[],
                        element_type (*)[]>::value,
                int> = 0>
        BLESSED_CXX14_CONSTEXPR explicit span(const Container &cont) noexcept : span(cont.data(), cont.size())
        {
        }

        template <
            std::size_t MyExtent = Extent, class Container,
            blessed::enable_if_t<
                MyExtent == dynamic_extent && std::is_const<element_type>::value &&
                    !detail::is_span<Container>::value && !detail::is_std_array<Container>::value &&
                    std::is_pointer<decltype(std::declval<const Container &>().data())>::value &&
                    std::is_convertible<
                        blessed::remove_pointer_t<decltype(std::declval<const Container &>().data())> (*)[],
                        element_type (*)[]>::value,
                int> = 0>
        BLESSED_CXX14_CONSTEXPR span(const Container &cont) noexcept : span(cont.data(), cont.size())
        {
        }

        BLESSED_CXX14_CONSTEXPR span(const span &other) noexcept = default;

        template <class OtherElementType, std::size_t OtherExtent, std::size_t MyExtent = Extent,
                  blessed::enable_if_t<(MyExtent == dynamic_extent || MyExtent == OtherExtent) &&
                                           detail::is_allowed_element_type_conversion<OtherElementType,
                                                                                      element_type>::value,
                                       int> = 0>
        BLESSED_CXX14_CONSTEXPR span(const span<OtherElementType, OtherExtent> &other) noexcept
            : storage_(other.data(), detail::extent_type<OtherExtent>(other.size()))
        {
        }

        template <class OtherElementType, std::size_t OtherExtent, std::size_t MyExtent = Extent,
                  blessed::enable_if_t<MyExtent != dynamic_extent && OtherExtent == dynamic_extent &&
                                           detail::is_allowed_element_type_conversion<OtherElementType,
                                                                                      element_type>::value,
                                       int> = 0>
        BLESSED_CXX14_CONSTEXPR explicit span(const span<OtherElementType, OtherExtent> &other) noexcept
            : storage_(other.data(), detail::extent_type<OtherExtent>(other.size()))
        {
        }

        ~span() noexcept = default;
        BLESSED_CXX14_CONSTEXPR span &operator=(const span &other) noexcept = default;

        // [span.sub], span subviews
        template <std::size_t Count>
        BLESSED_CXX14_CONSTEXPR span<element_type, Count> first() const noexcept
        {
            assert(Count <= size());
            return span<element_type, Count>{data(), Count};
        }

        template <std::size_t Count>
        BLESSED_CXX14_CONSTEXPR span<element_type, Count> last() const noexcept
        {
            assert(Count <= size());
            return span<element_type, Count>{data() + (size() - Count), Count};
        }

        template <std::size_t Offset, std::size_t Count = dynamic_extent>
        BLESSED_CXX14_CONSTEXPR auto subspan() const noexcept -> typename detail::calculate_subspan_type<ElementType, Extent, Offset, Count>::type
        {
            assert((size() >= Offset) && (Count == dynamic_extent || (Count <= size() - Offset)));
            using type =
                typename detail::calculate_subspan_type<ElementType, Extent, Offset, Count>::type;
            return type{data() + Offset, Count == dynamic_extent ? size() - Offset : Count};
        }

        BLESSED_CXX14_CONSTEXPR span<element_type, dynamic_extent> first(size_type count) const noexcept
        {
            assert(count <= size());
            return {data(), count};
        }

        BLESSED_CXX14_CONSTEXPR span<element_type, dynamic_extent> last(size_type count) const noexcept
        {
            assert(count <= size());
            return make_subspan(size() - count, dynamic_extent, subspan_selector<Extent>{});
        }

        BLESSED_CXX14_CONSTEXPR span<element_type, dynamic_extent>
        subspan(size_type offset, size_type count = dynamic_extent) const noexcept
        {
            return make_subspan(offset, count, subspan_selector<Extent>{});
        }

        // [span.obs], span observers
        BLESSED_CXX14_CONSTEXPR size_type size() const noexcept { return storage_.size(); }

        BLESSED_CXX14_CONSTEXPR size_type size_bytes() const noexcept
        {
            assert(size() < dynamic_extent / sizeof(element_type));
            return size() * sizeof(element_type);
        }

        BLESSED_CXX14_CONSTEXPR bool empty() const noexcept { return size() == 0; }

        // [span.elem], span element access
        BLESSED_CXX14_CONSTEXPR reference operator[](size_type idx) const noexcept
        {
            assert(idx < size());
            return data()[idx];
        }

        BLESSED_CXX14_CONSTEXPR reference front() const noexcept
        {
            assert(size() > 0);
            return data()[0];
        }

        BLESSED_CXX14_CONSTEXPR reference back() const noexcept
        {
            assert(size() > 0);
            return data()[size() - 1];
        }

        BLESSED_CXX14_CONSTEXPR pointer data() const noexcept { return storage_.data(); }

        // [span.iter], span iterator support
        BLESSED_CXX14_CONSTEXPR iterator begin() const noexcept
        {
            const auto data = storage_.data();

            return {data, data + size(), data};
        }

        BLESSED_CXX14_CONSTEXPR iterator end() const noexcept
        {
            const auto data = storage_.data();

            const auto endData = data + storage_.size();
            return {data, endData, endData};
        }

        BLESSED_CXX14_CONSTEXPR reverse_iterator rbegin() const noexcept { return reverse_iterator{end()}; }
        BLESSED_CXX14_CONSTEXPR reverse_iterator rend() const noexcept { return reverse_iterator{begin()}; }

#ifdef _MSC_VER
        // Tell MSVC how to unwrap spans in range-based-for
        constexpr pointer _Unchecked_begin() const noexcept { return data(); }
        constexpr pointer _Unchecked_end() const noexcept
        {
            return data() + size();
        }
#endif // _MSC_VER

    private:
        // Needed to remove unnecessary null check in subspans
        struct KnownNotNull
        {
            pointer p;
        };

        // this implementation detail class lets us take advantage of the
        // empty base class optimization to pay for only storage of a single
        // pointer in the case of fixed-size spans
        template <class ExtentType>
        class storage_type : public ExtentType
        {
        public:
            // KnownNotNull parameter is needed to remove unnecessary null check
            // in subspans and constructors from arrays
            template <class OtherExtentType>
            BLESSED_CXX14_CONSTEXPR storage_type(KnownNotNull data, OtherExtentType ext)
                : ExtentType(ext), data_(data.p)
            {
            }

            template <class OtherExtentType>
            BLESSED_CXX14_CONSTEXPR storage_type(pointer data, OtherExtentType ext) : ExtentType(ext), data_(data)
            {
                assert(data || ExtentType::size() == 0);
            }

            BLESSED_CXX14_CONSTEXPR pointer data() const noexcept { return data_; }

        private:
            pointer data_;
        };

        storage_type<detail::extent_type<Extent>> storage_;

        // The rest is needed to remove unnecessary null check
        // in subspans and constructors from arrays
        BLESSED_CXX14_CONSTEXPR span(KnownNotNull ptr, size_type count) noexcept : storage_(ptr, count) {}

        template <std::size_t CallerExtent>
        class subspan_selector
        {
        };

        template <std::size_t CallerExtent>
        BLESSED_CXX14_CONSTEXPR span<element_type, dynamic_extent>
        make_subspan(size_type offset, size_type count, subspan_selector<CallerExtent>) const noexcept
        {
            const span<element_type, dynamic_extent> tmp(*this);
            return tmp.subspan(offset, count);
        }

        BLESSED_CXX14_CONSTEXPR span<element_type, dynamic_extent>
        make_subspan(size_type offset, size_type count, subspan_selector<dynamic_extent>) const noexcept
        {
            assert(size() >= offset);

            if (count == dynamic_extent)
            {
                return {KnownNotNull{data() + offset}, size() - offset};
            }

            assert(size() - offset >= count);
            return {KnownNotNull{data() + offset}, count};
        }
    };

#if (defined(__cpp_deduction_guides) && (__cpp_deduction_guides >= 201611L))

    // Deduction Guides
    template <class Type, std::size_t Extent>
    span(Type (&)[Extent]) -> span<Type, Extent>;

    template <class Type, std::size_t Size>
    span(std::array<Type, Size> &) -> span<Type, Size>;

    template <class Type, std::size_t Size>
    span(const std::array<Type, Size> &) -> span<const Type, Size>;

    template <class Container, class Element = blessed::remove_pointer_t<decltype(std::declval<Container &>().data())>>
    span(Container &) -> span<Element>;

    template <class Container, class Element = blessed::remove_pointer_t<decltype(std::declval<const Container &>().data())>>
    span(const Container &) -> span<Element>;

#endif // ( defined(__cpp_deduction_guides) && (__cpp_deduction_guides >= 201611L) )

#if defined(GSL_USE_STATIC_CONSTEXPR_WORKAROUND)
    template <class ElementType, std::size_t Extent>
    constexpr const typename span<ElementType, Extent>::size_type span<ElementType, Extent>::extent;
#endif

    namespace detail
    {
        // if we only supported compilers with good constexpr support then
        // this pair of classes could collapse down to a constexpr function

        // we should use a narrow_cast<> to go to std::size_t, but older compilers may not see it as
        // constexpr
        // and so will fail compilation of the template
        template <class ElementType, std::size_t Extent>
        struct calculate_byte_size : std::integral_constant<std::size_t, sizeof(ElementType) * Extent>
        {
            static_assert(Extent < dynamic_extent / sizeof(ElementType), "Size is too big.");
        };

        template <class ElementType>
        struct calculate_byte_size<ElementType, dynamic_extent>
            : std::integral_constant<std::size_t, dynamic_extent>
        {
        };
    }

    template <class ElementType, std::size_t Extent>
    span<const byte, detail::calculate_byte_size<ElementType, Extent>::value>
    as_bytes(span<ElementType, Extent> s) noexcept
    {
        using type = span<const byte, detail::calculate_byte_size<ElementType, Extent>::value>;

        return type{reinterpret_cast<const byte *>(s.data()), s.size_bytes()};
    }

    template <class ElementType, std::size_t Extent,
              blessed::enable_if_t<!std::is_const<ElementType>::value, int> = 0>
    span<byte, detail::calculate_byte_size<ElementType, Extent>::value>
    as_writable_bytes(span<ElementType, Extent> s) noexcept
    {
        using type = span<byte, detail::calculate_byte_size<ElementType, Extent>::value>;

        return type{reinterpret_cast<byte *>(s.data()), s.size_bytes()};
    }

}

#endif

namespace blessed
{
    template<typename _Type, typename _OffsetType>
    _Type *reinterpret_as(span<blessed::byte> s, _OffsetType offset) noexcept
    {
        if((sizeof(_Type) + offset) <= s.size_bytes())
        {
            return reinterpret_cast<_Type *>(&s[offset]);
        }

        return nullptr;
    }

    template<typename _Type>
    span<_Type> reinterpret_as(span<blessed::byte> s) noexcept
    {
        return span<_Type>(reinterpret_as<_Type>(s, 0), s.size_bytes() / sizeof(_Type));
    }
    
    template<typename _Type>
    span<blessed::byte> reinterpret_as(span<blessed::byte> s, _Type *ptr, size_t sz) noexcept
    {
        auto offset = reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(s.data());

        if((sizeof(_Type) + offset) <= s.size_bytes())
        {
            return s.subspan(offset, sizeof(_Type));
        }

        return span<blessed::byte>{};
    }
}

#endif
