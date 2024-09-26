#pragma once

#include <assert.h>
#include <bit>
#include <concepts>
#include <numeric>
#include <utility>

namespace tp
{

    namespace platform
    {
#if defined(__x86_64__) || defined(__x86_64) || defined(_M_AMD64)
        constexpr size_t pointer_high_bits_reserve = 16;
        constexpr size_t pointer_high_bits_fill = 0;
#else//todo other platforms
        constexpr size_t pointer_high_bits_reserve = 0;
        constexpr size_t pointer_high_bits_fill = 0;
#endif
#if defined(__x86_64__) || defined(__x86_64) || defined(_M_AMD64)
        constexpr size_t function_align = 16;
#else //todo other platforms
        constexpr size_t function_align = 2;
#endif

    }

    namespace detail
    {
        constexpr bool is_pow_of_2(size_t val)
        {
            return val && !(val & (val - 1));
        }


        template<typename T>
        constexpr size_t alignof_extention()
        {
            constexpr bool is_func = std::is_function_v<T> || std::is_member_function_pointer_v<T>;
            if constexpr (is_func)
                return platform::function_align;
            else if constexpr (std::is_void_v<T>)
                return 1;
            else
            {
                static_assert(is_pow_of_2(alignof(T)));
                return alignof(T);
            }
        }

#undef FUNCTION_ALIGN

    }// namespace detail



    template<typename T>
    class tagged_ptr_base
    {
    protected:
        static constexpr size_t align = detail::alignof_extention<T>();
        static constexpr size_t low_bits = std::countr_zero(align);
        static constexpr size_t high_bits = platform::pointer_high_bits_reserve;
        static constexpr uintptr_t pointer_high_bits_fill = platform::pointer_high_bits_fill;
        static constexpr uintptr_t low_bits_mask = align - 1;
        static constexpr uintptr_t high_bits_mask = ~(((~uintptr_t(0)) << high_bits) >> high_bits);
        static constexpr uintptr_t pointer_bit_mask = ~(low_bits_mask | high_bits_mask);
        static constexpr size_t max_bits = low_bits + high_bits;

        uintptr_t _ptr;

        T* get_ptr() const
        {
            return reinterpret_cast<T*>(_ptr & pointer_bit_mask);
        }

        void set_ptr(T* ptr)
        {
            assert((uintptr_t(ptr) & low_bits_mask) == 0);
            _ptr = _ptr & high_bits_mask | _ptr & low_bits_mask | ptr & pointer_bit_mask;
        }

        void set_low_bits(uintptr_t bits)
        {
            assert((uintptr_t(bits) & (~low_bits_mask)) == 0);
            _ptr = _ptr & (~low_bits_mask) | bits;
        }

        void set_high_bits(uintptr_t bits)
        {
            assert((uintptr_t(bits) & (~high_bits_mask)) == 0);
            _ptr = _ptr & (~high_bits_mask) | bits;
        }

        template<size_t I>
        requires(I < max_bits)
        constexpr std::pair<uintptr_t, size_t> tag_mask()
        {
            if constexpr (I < low_bits)
            {
                constexpr auto mask = uintptr_t(1) << I;
                return {mask, I};
            } else//high bits
            {
                constexpr size_t high_bit_inv_pos = I - low_bits;
                constexpr size_t high_bit_pos = sizeof(uintptr_t) * 8 - 1 - high_bit_inv_pos;
                constexpr uintptr_t mask = uintptr_t(1) << high_bit_pos;
                return {mask, high_bit_pos};
            }
        }

        template<size_t I>
        requires(I < max_bits)
        constexpr bool get_tag()
        {
            constexpr auto mask = tag_mask().first;
            return _ptr & mask;
        }

        template<size_t I>
        requires(I < max_bits)
        constexpr void set_tag(bool val)
        {
            constexpr auto mask = tag_mask<I>().first;
            constexpr auto shift = tag_mask<I>().second;
            _ptr = _ptr & (~mask) | (uintptr_t(val) << shift);
        }

        template<size_t Begin, size_t Bits>
        static constexpr uintptr_t sequence_mask()
        {
            constexpr size_t r_zeros = Begin;
            constexpr size_t l_zeros = sizeof(uintptr_t) * 8 - (Begin + Bits);
            constexpr uintptr_t mask_r = ((~uintptr_t(0)) >> r_zeros) << r_zeros;
            constexpr uintptr_t mask = (mask_r << l_zeros) >> l_zeros;
            return mask;
        }

    public:
        template<typename IntType, size_t Begin, size_t Bits>
        constexpr IntType get_int() const
        {
            constexpr uintptr_t mask = sequence_mask<Begin, Bits>();
            static_assert((mask & pointer_bit_mask) == 0);
            uintptr_t i = (_ptr & mask) >> Begin;
            return (IntType) (i);
        }

        template<typename IntType, size_t Begin, size_t Bits>
        constexpr void set_int(IntType val)
        {
            static constexpr uintptr_t mask = sequence_mask<Begin, Bits>();
            static_assert((mask & pointer_bit_mask) == 0);
            uintptr_t i = (uintptr_t) (val) << Begin;
            _ptr = _ptr & (~mask) | i;
        }

    private:
        template<typename U>
        struct add_ref
        {
            using type = U&;
        };
        template<>
        struct add_ref<void>
        {
            using type = void;
        };
        using reference_type = add_ref<T>::type;

    public:
        tagged_ptr_base() : _ptr(0) {}

        tagged_ptr_base(T* ptr) : _ptr((uintptr_t) ptr)
        {
            assert((_ptr & ~pointer_bit_mask) == 0);
        }

        reference_type operator*()
        {
            if constexpr (!std::same_as<T, void>)
                return *get_ptr();
        }

        T* operator->()
        {
            return get_ptr();
        }

        operator bool()
        {
            return get_ptr() != nullptr;
        }

        tagged_ptr_base& operator=(T* ptr)
        {
            set_ptr(ptr);
            return *this;
        }

        void reset_tags()
        {
            _ptr = _ptr & pointer_bit_mask;
        }
    };


    template<typename PtrT, typename T, size_t Begin, size_t Bits>
    struct tag_ref
    {
    private:
        tagged_ptr_base<PtrT>* ptr;

    public:
        tag_ref(tagged_ptr_base<PtrT>* ptr) : ptr(ptr) {}

        tag_ref operator=(const T& val)
        {
            ptr->template set_int<T, Begin, Bits>(val);
            return *this;
        }

        T get() const { return ptr->template get_int<T, Begin, Bits>(); }

        operator T() const
        {
            return get();
        }
    };

    template<typename T, size_t Bits>
    struct tag_of {};

    namespace detail
    {
        template<typename U>
        struct tag_type_t { using type = U; };
        template<typename U, size_t Bits>
        struct tag_type_t<tag_of<U, Bits>> { using type = U; };

        template<typename U>
        using tag_type = detail::tag_type_t<U>::type;
    }


    template<typename T, typename... Tags>
    class tagged_ptr : public tagged_ptr_base<T>
    {
        using base = tagged_ptr_base<T>;

        template<typename U>
        struct tag_size
        {
            static constexpr size_t value = sizeof(U) * 8;
        };
        template<>
        struct tag_size<bool>
        {
            static constexpr size_t value = 1;
        };
        template<typename U, size_t Bits>
        struct tag_size<tag_of<U, Bits>>
        {
            static constexpr size_t value = Bits;
        };


        template<size_t... I>
        static constexpr size_t sum()
        {
            if constexpr (sizeof...(I) == 0)
                return 0;
            else
                return (I + ...);
        }

        static constexpr std::array<size_t, sizeof...(Tags)> tags_size = {tag_size<Tags>::value...};
        static constexpr std::array<size_t, sizeof...(Tags)> tags_begin = []
        {
            std::array<size_t, sizeof...(Tags)> arr;
            size_t current = 0;
            for (size_t i = 0; i < sizeof...(Tags); ++i)
            {
                if (current + tags_size[i] < base::low_bits)
                {
                    arr[i] = current;
                    current += tags_size[i];
                }
                else
                {
                    current = sizeof(uintptr_t) * 8 - base::high_bits;
                    arr[i] = current;
                    current += tags_size[i];
                }
            }
            return arr;
        }();
        static constexpr size_t tag_bits = sum<tag_size<Tags>::value...>();
        static_assert(tag_bits <= base::max_bits);

    public:
        tagged_ptr() : base() {}

        tagged_ptr(T* ptr) : base(ptr) {}

        template<size_t I>
        auto get()
        {
            using tag = std::tuple_element<I, std::tuple<Tags...>>::type;
            using type = detail::tag_type<tag>;
            return tag_ref<T, type, tags_begin[I], tags_size[I]>(this);
        }

        template<size_t I>
        auto get() const
        {
            using tag = std::tuple_element<I, std::tuple<Tags...>>::type;
            using type = detail::tag_type<tag>;
            return base::template get_int<type, tags_begin[I], tags_size[I]>();
        }

        T* get()
        {
            return base::get_ptr();
        }
    };

}// namespace tp

namespace std
{
    template<typename T, typename... Tags>
    struct tuple_size<tp::tagged_ptr < T, Tags...>> : std::integral_constant<std::size_t, sizeof...(Tags)>
{
};

template<std::size_t Index, typename T, typename... Tags>
struct tuple_element<Index, tp::tagged_ptr<T, Tags...>>
{
    using type = decltype(std::declval<tp::tagged_ptr<T, Tags...>>().template get<Index>());
};

template<std::size_t Index, typename T, typename... Tags>
struct tuple_element<Index, const tp::tagged_ptr<T, Tags...>>
{
    using type = decltype(std::declval<const tp::tagged_ptr<T, Tags...>>().template get<Index>());
};
}