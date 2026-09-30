#pragma once

#include <justaman/memory/lifetime.hpp>
#include <justaman/utils/attributes.h>
#include <justaman/algorithms/numeric.hpp>
#include <justaman/utils/scope.hpp>
#include <justaman/errors/safe_exceptions.hpp>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <type_traits>
#include <span>

namespace jman::detail {
    template <typename T>
    struct vector_const_iterator;

    template <typename T>
    struct vector_iterator;
} // namespace jman::detail

namespace jman
{
    /**
        @brief
            My custom implementation for a simpler version for std::vector.
            Tried to make it as accuarate as the standard version while giving more flexibility

        @todo
            add memcpy and memmove when types r trivially copyable and movable
    */
    template <typename T, typename S = std::size_t>
    class vector
    {
        static_assert(!std::is_reference_v<T>, "Type T can't be a reference of any kind.");
        static_assert(!std::is_void_v<T>, "Type T can't be a of void type.");

        static_assert(std::is_nothrow_destructible_v<T>, "Type T must has it's destructor marked as noexcept for safety.");
        static_assert(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>, "Type T must be move or copy constructible.");
        static_assert(std::is_move_assignable_v<T> || std::is_copy_assignable_v<T>, "Type T must be move or copy assignable.");

        static_assert(std::is_integral_v<S> && std::is_unsigned_v<S>, "Size type S must be an unsigned integral.");

        public:
            using value_type            = T;
            using size_type             = S;
            using difference_type       = std::ptrdiff_t;
            using reference             = T&;
            using const_reference       = const T&;
            using pointer               = T*;
            using const_pointer         = const T*;

            using iterator                  = detail::vector_iterator<T>;
            using const_iterator            = detail::vector_const_iterator<T>;

            using reverse_iterator          = std::reverse_iterator<iterator>;
            using const_reverse_iterator    = std::reverse_iterator<const_iterator>;

            constexpr static size_type MIN_CAPACITY     = 16;
            constexpr static size_type GROW_FACTOR      = 2;

            // ############################################################
            // -------------------------------- constructors / destructors
            // ##########################################################

            vector() noexcept = default;
            vector(std::initializer_list<T> list);
            explicit vector(size_type size);
            vector(size_type size, const_reference value);

            vector(const vector& other);
            vector& operator=(const vector& other);

            vector(vector&& other) noexcept;
            vector& operator=(vector&& other) noexcept;

            vector& operator=(std::initializer_list<T> list);

            ~vector() noexcept;

            // ##################################################
            // ------------------------------------------- stats
            // ################################################

            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type         capacity() const noexcept   { return m_capacity; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type         remaining() const noexcept  { return m_capacity - m_size; }
            /** @warning returned value is an aproximation. */
            JMAN_NO_DISCARD JMAN_FORCE_INLINE static size_type  max_size() noexcept         { return std::numeric_limits<size_type>::max() / sizeof(T); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type         size() const noexcept       { return m_size; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool              empty() const noexcept      { return m_size == 0; }

            // #####################################################
            // ------------------------------------------ accessing
            // ###################################################

            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_pointer             data() const noexcept                       { return std::launder(m_buffer); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE pointer                   data() noexcept                             { return std::launder(m_buffer); }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference           front() const noexcept                      { return m_buffer[0]; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference           back() const noexcept                       { return m_buffer[m_size - 1]; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference                 front() noexcept                            { return m_buffer[0]; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference                 back() noexcept                             { return m_buffer[m_size - 1]; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference                 operator[](size_type index) noexcept        { return m_buffer[index]; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference           operator[](size_type index) const noexcept  { return m_buffer[index]; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::span<T>              slice(size_t offset, size_t count)  noexcept        { return {m_buffer + offset, count}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::span<T>              left(size_t count) noexcept                         { return {m_buffer, count}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::span<T>              right(size_t count) noexcept                        { return {m_buffer + m_size - count, count}; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE const std::span<T>        slice(size_t offset, size_t count) const noexcept   { return {m_buffer + offset, count}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const std::span<T>        left(size_t count) const noexcept                   { return {m_buffer, count}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const std::span<T>        right(size_t count) const noexcept                  { return {m_buffer + m_size - count, count}; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference                 front_or(reference fallback) noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference                 back_or(reference fallback) noexcept;

            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference           front_or(reference fallback) const noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference           back_or(reference fallback) const noexcept;

            JMAN_NO_DISCARD reference                                   at(size_type index);
            JMAN_NO_DISCARD const_reference                             at(size_type index) const;

            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator                  find(size_type index) noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator            find(size_type index) const noexcept;

            JMAN_NO_DISCARD bool                                        contains(const_reference value) const noexcept(noexcept(std::declval<T>() == std::declval<T>()));

            // ################################################
            // ------------------------------------- iterators
            // ##############################################

            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator                  begin() noexcept            { return iterator{m_buffer}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator                  end() noexcept              { return iterator{m_buffer + m_size}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator            begin() const noexcept      { return const_iterator{m_buffer}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator            end() const noexcept        { return const_iterator{m_buffer + m_size}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator            cbegin() const noexcept     { return const_iterator {m_buffer}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator            cend() const noexcept       { return const_iterator{m_buffer + m_size}; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE reverse_iterator          rbegin() noexcept           { return reverse_iterator{iterator{m_buffer}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reverse_iterator          rend() noexcept             { return reverse_iterator{iterator{m_buffer + m_size}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    rbegin() const noexcept     { return const_reverse_iterator{const_iterator{m_buffer}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    rend() const noexcept       { return const_reverse_iterator{const_iterator{m_buffer + m_size}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    crbegin() const noexcept    { return const_reverse_iterator{const_iterator {m_buffer}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    crend() const noexcept      { return const_reverse_iterator{const_iterator{m_buffer + m_size}}; }

            // ##################################################
            // -------------------------------- memory modifiers
            // ################################################

            void                                                reserve(size_type count);
            void                                                set_capacity(size_type capacity);

            /** @brief clears elements and frees the heap memory */
            void                                                reset() noexcept;
            void                                                clear() noexcept;

            void                                                resize(size_type newSize);
            void                                                resize(size_type newSize, const_reference value); // must be copyable

            void                                                shrink_to_fit();
            void                                                swap(vector& other) noexcept;

            // ####################################################
            // ---------------------------------------- algorithms
            // ##################################################

            template <std::predicate<T> Predicate>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator          find_first_if(Predicate pred) noexcept(std::is_nothrow_invocable_r_v<bool, Predicate, const T&>);
            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator          find_first(const_reference) noexcept(noexcept(std::declval<T>() == std::declval<T>()));

            template <std::predicate<T> Predicate>
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator    find_first_if(Predicate pred) const noexcept(std::is_nothrow_invocable_r_v<bool, Predicate, const T&>);
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator    find_first(const_reference) const noexcept(noexcept(std::declval<T>() == std::declval<T>()));

            template <std::predicate<T> Predicate>
            void                                                remove_if(Predicate pred) noexcept(!m_needs_rollback_on_assign && std::is_nothrow_invocable_r_v<bool, Predicate, const T&>);
            void                                                remove(const_reference value) noexcept(!m_needs_rollback_on_assign && noexcept(std::declval<T>() == std::declval<T>()));
            void                                                reverse() noexcept(std::is_nothrow_swappable_v<T>);
            size_type                                           unique() noexcept(!m_needs_rollback_on_assign && noexcept(std::declval<T>() == std::declval<T>()));
            template <std::predicate<T, T> Predicate>
            size_type                                           unique(Predicate pred) noexcept(!m_needs_rollback_on_assign && std::is_nothrow_invocable_r_v<bool, Predicate, const T&>);
            template <std::predicate<T> Predicate>
            void                                                retain(Predicate pred) noexcept(!m_needs_rollback_on_assign && std::is_nothrow_invocable_r_v<bool, Predicate, const T&>);

            // ####################################################
            // -------------------------------- elements insertion
            // ##################################################

            template <typename ...Args> requires std::is_constructible_v<T, Args...>
            void                                                emplace_back(Args&&... args);
            template <typename TT>      requires std::is_convertible_v<TT, T>
            void                                                push_back(TT&& value);

            template <typename TT>      requires std::is_convertible_v<TT, T>
            void                                                insert(size_type index, TT&& value);
            void                                                insert(size_type index, size_type count, const_reference value);
            void                                                insert(size_type index, std::initializer_list<T> list);
            template <typename ...Args> requires std::is_constructible_v<T, Args...>
            void                                                emplace(size_type index, Args&&... args);

            template <typename TT>      requires std::is_convertible_v<TT, T>
            iterator                                            insert(const_iterator it, TT&& value);
            iterator                                            insert(const_iterator it, size_type count, const_reference value);
            iterator                                            insert(const_iterator it, std::initializer_list<T> list);
            template <typename ...Args>
            iterator                                            emplace(const_iterator it, Args&&... args);

            void                                                assign(size_type count, const_reference value);

            iterator                                            append(const vector& vec);
            iterator                                            append(vector&& vec);

            void                                                append(size_type index, const vector& vec);
            void                                                append(size_type index, vector&& vec);

            iterator                                            append(const_iterator it, const vector& vec);
            iterator                                            append(const_iterator it, vector&& vec);

            // ####################################################
            // ---------------------------------- elements removal
            // ##################################################

            void                                                pop_back() noexcept;

            size_type                                           erase(size_type index);
            /** @brief deletes elements from @first and stops when reaching @last (@last doesnt get deleted) */
            size_type                                           erase(size_type first, size_type last);

            iterator                                            erase(const_iterator it);
            /** @brief deletes elements from @first and stops when reaching @last (@last doesnt get deleted) */
            iterator                                            erase(const_iterator first, const_iterator last);

            // ######################################################
            // ---------------------------------- private class impl
            // ####################################################

            protected:
                T*          m_buffer    {nullptr};

                size_type   m_capacity  {0};
                size_type   m_size      {0};

                constexpr static bool m_needs_rollback_on_construct =
                    !std::is_nothrow_move_constructible_v<T> && !std::is_nothrow_copy_constructible_v<T>;

                constexpr static bool m_needs_rollback_on_assign =
                    (!std::is_nothrow_move_assignable_v<T> && !std::is_nothrow_copy_assignable_v<T>);

                constexpr static bool m_is_trivial_type =           std::is_trivially_copyable_v<T>;
                constexpr static bool m_is_good_size =              sizeof(T) == 1 || sizeof(T) == 2 ||
                                                                        sizeof(T) == 4 || sizeof(T) == 8 ||
                                                                        sizeof(T) == 16;
#if defined (JMAN_HARDWARE_SUPPORTS_SIMD)
                constexpr static bool m_hardware_supports_simd =        true;
#else
                constexpr static bool m_hardware_supports_simd =        false;
#endif

            protected:
                // shortcuts for calculating calculating capacities for when calling m_alloc()
                // that throw when any operation overflows the size_type specified
                JMAN_NO_DISCARD size_type           m_growth_capacity(size_type required) const;
                JMAN_NO_DISCARD size_type           m_required_capacity(size_type increase) const;

                // shortcut for allocating and deallocating blocks of mem
                JMAN_NO_DISCARD static T*           m_alloc(size_type capacity);
                JMAN_FORCE_INLINE static void       m_dealloc(T* ptr) noexcept;

                // shortcut for growing the vector
                void                                m_grow(size_type increase = 1);

                void                                m_shift_right(size_type index, size_type count) noexcept(!m_needs_rollback_on_construct && !m_needs_rollback_on_assign);
                // note that this func leaves the tail of undefined but still constructed elements, so the caller function must call its destructors
                void                                m_shift_left(size_type index, size_type count) noexcept(!m_needs_rollback_on_assign);

                template <bool Move>
                void                                m_insert_range(size_type index, T* src, size_type count);

    }; // class vector
}; // namespace jman

// #####################################################################################################################
// --------------------------------------------------- ITERATORS IMPL ------------------------------------------------//
// #####################################################################################################################

namespace jman::detail
{
    template <typename T>
    struct vector_const_iterator
    {
        private:
            const T* m_ptr = nullptr;

        public:
            using iterator_concept      = std::contiguous_iterator_tag;
            using iterator_category     = std::random_access_iterator_tag;
            using value_type            = T;
            using difference_type       = std::ptrdiff_t;
            using reference             = const T&;
            using pointer               = const T*;

            vector_const_iterator() noexcept = default;
            explicit vector_const_iterator(T* ptr) noexcept: m_ptr(ptr) {}

            JMAN_FORCE_INLINE
            reference operator[](difference_type index) const noexcept                                { return m_ptr[index]; }

            JMAN_FORCE_INLINE
            vector_const_iterator&    operator++() noexcept                                           { m_ptr += 1; return *this; }
            JMAN_FORCE_INLINE
            vector_const_iterator     operator++(int) noexcept                                        { vector_const_iterator retval = *this; ++(*this); return retval; }
            JMAN_FORCE_INLINE
            vector_const_iterator&    operator--() noexcept                                           { m_ptr -= 1; return *this; }
            JMAN_FORCE_INLINE
            vector_const_iterator     operator--(int) noexcept                                        { vector_const_iterator retval = *this; --(*this); return retval; }

            JMAN_FORCE_INLINE
            vector_const_iterator     operator+(difference_type index) const noexcept                 { return vector_const_iterator(m_ptr + index); }
            JMAN_FORCE_INLINE
            vector_const_iterator     operator-(difference_type index) const noexcept                 { return vector_const_iterator(m_ptr - index); }

            JMAN_FORCE_INLINE
            difference_type           operator-(const vector_const_iterator& other) const noexcept          { return m_ptr - other.m_ptr; }

            JMAN_FORCE_INLINE
            vector_const_iterator&    operator+=(difference_type diff) noexcept                       { m_ptr += diff; return *this; }
            JMAN_FORCE_INLINE
            vector_const_iterator&    operator-=(difference_type diff) noexcept                       { m_ptr -= diff; return *this; }
            JMAN_FORCE_INLINE friend  vector_const_iterator     operator+(difference_type diff, vector_const_iterator it) noexcept     { return {diff + it.m_ptr}; }

            JMAN_FORCE_INLINE
            pointer                   operator->() const noexcept  { return m_ptr; }
            JMAN_FORCE_INLINE
            reference                 operator*() const noexcept   { return *m_ptr; }

            auto operator <=>(const vector_const_iterator&) const = default;
    }; // vector_const_iterator

    template <typename T>
    struct vector_iterator
    {
        private:
            T* m_ptr = nullptr;

        public:
            using iterator_concept      = std::contiguous_iterator_tag;
            using iterator_category     = std::random_access_iterator_tag;
            using value_type            = T;
            using difference_type       = std::ptrdiff_t;
            using pointer               = T*;
            using reference             = T&;

            vector_iterator() noexcept = default;
            explicit vector_iterator(T* ptr) noexcept: m_ptr(ptr) {}

            JMAN_FORCE_INLINE
            vector_iterator&              operator++() noexcept                                   { ++m_ptr; return *this; }
            JMAN_FORCE_INLINE
            vector_iterator               operator++(int) noexcept                                { vector_iterator retval = *this; ++(*this); return retval; }
            JMAN_FORCE_INLINE
            vector_iterator&              operator--() noexcept                                   { --m_ptr; return *this; }
            JMAN_FORCE_INLINE
            vector_iterator               operator--(int) noexcept                                { vector_iterator retval = *this; --(*this); return retval; }

            JMAN_FORCE_INLINE
            reference                     operator[](difference_type index) const noexcept        { return m_ptr[index]; }

            JMAN_FORCE_INLINE
            vector_iterator               operator+(difference_type index) const noexcept         { return vector_iterator(m_ptr + index); }
            JMAN_FORCE_INLINE
            vector_iterator               operator-(difference_type index) const noexcept         { return vector_iterator(m_ptr - index); }

            JMAN_FORCE_INLINE
            difference_type               operator-(const vector_iterator& other) const noexcept         { return m_ptr - other.m_ptr; }

            JMAN_FORCE_INLINE
            vector_iterator&              operator+=(difference_type diff) noexcept               { m_ptr += diff; return *this; }
            JMAN_FORCE_INLINE
            vector_iterator&              operator-=(difference_type diff) noexcept               { m_ptr -= diff; return *this; }
            JMAN_FORCE_INLINE friend vector_iterator   operator+(difference_type diff, vector_iterator it) noexcept   { return {diff + it.m_ptr}; }

            JMAN_FORCE_INLINE
            pointer                       operator->() const noexcept         { return m_ptr; }
            JMAN_FORCE_INLINE
            reference                     operator*() const noexcept          { return *m_ptr; }

            auto operator <=>(const vector_iterator&) const noexcept          = default;
            operator vector_const_iterator<T>() const  noexcept               { return {m_ptr}; };
    }; // vector_iterator

} // namespace jman::detail

namespace jman
{

// #####################################################################################################################
// ---------------------------------------------- PRIVATE / HIDDEN METHODS -------------------------------------------//
// #####################################################################################################################

template <typename T, typename S>
JMAN_NO_DISCARD vector<T, S>::size_type vector<T, S>::m_growth_capacity(size_type required) const
{
    size_type result;
    if JMAN_UNLIKELY(!jman::validate_mul<size_type>(m_capacity, GROW_FACTOR, &result))
        JMAN_THROW(std::overflow_error("jman::vector<T, S>::m_growth_capacity() failed: calculated capacity overflows the size_type specified."));

    size_type newCapacity = jman::max(result, required);
    return jman::max(newCapacity, MIN_CAPACITY);
}

template <typename T, typename S>
JMAN_NO_DISCARD vector<T, S>::size_type vector<T, S>::m_required_capacity(size_type increase) const
{
    size_type result;
    if JMAN_UNLIKELY(!jman::validate_add<size_type>(m_size, increase, &result))
        JMAN_THROW(std::overflow_error("jman::vector<T, S>::m_required_capacity() failed: calculated minimum required capacity overflows the size_type specified."));

    return result;
}




template <typename T, typename S>
JMAN_NO_DISCARD T* vector<T, S>::m_alloc(size_type capacity)
{
    size_type result;
    if JMAN_UNLIKELY(!jman::validate_mul<size_type>(sizeof(T), capacity, &result))
        JMAN_THROW(std::overflow_error("jman::vector<T, S>::m_alloc() failed: calculated capacity overflows the size_type specified."));

    return static_cast<T*>(::operator new(result, std::align_val_t{alignof(T)}));
}

template <typename T, typename S>
JMAN_FORCE_INLINE void vector<T, S>::m_dealloc(T* ptr) noexcept
{
    ::operator delete(static_cast<void*>(ptr), std::align_val_t{alignof(T)});
}





template <typename T, typename S>
void vector<T, S>::m_shift_right(size_type index, size_type count)
    noexcept(!m_needs_rollback_on_construct && !m_needs_rollback_on_assign)
{
    size_type elements_to_move = m_size - index;
    if (elements_to_move == 0) return;

    if constexpr (m_is_trivial_type)
        std::memmove(m_buffer + index + count, m_buffer + index, sizeof(T) * elements_to_move);
    else
    {
        size_type moved = 0;
        auto guard = jman::scope_guard{[&](){
            if constexpr (
                m_needs_rollback_on_assign ||
                m_needs_rollback_on_construct
            )
                for (size_type i = 0; i < moved; ++i) {
                    size_type current_idx = m_size + count - i;
                    size_type original_idx = current_idx - count;

                    m_buffer[original_idx] = std::move_if_noexcept(m_buffer[current_idx]);
                    if (current_idx >= m_size) jman::destruct_at(m_buffer + current_idx);
                }
        }};

        for (size_type i = elements_to_move; i-- > 0;) {
            size_type src_idx = index + i;
            size_type dst_idx = src_idx + count;

            if (dst_idx >= m_size)
                jman::construct_at(m_buffer + dst_idx, std::move_if_noexcept(m_buffer[src_idx]));
            else
                m_buffer[dst_idx] = std::move_if_noexcept(m_buffer[src_idx]);

            ++moved;
        }

        guard.dismiss();
    }
}
template <typename T, typename S>
void vector<T, S>::m_shift_left(size_type index, size_type count)
    noexcept(!m_needs_rollback_on_assign)
{
    size_type elements_to_move = m_size - (index + count);
    if (elements_to_move == 0) return;

    if constexpr (m_is_trivial_type)
        std::memmove(m_buffer + index, m_buffer + index + count, sizeof(T) * elements_to_move);
    else
    {
        size_type moved = 0;

        auto guard = jman::scope_guard{[&](){
            if constexpr (m_needs_rollback_on_assign)
                for (size_type i = moved; i-- > 0;) {
                    size_type current_idx = index + i;
                    size_type original_idx = current_idx + count;

                    m_buffer[original_idx] = std::move_if_noexcept(m_buffer[current_idx]);
                }
        }};

        for (size_type i = 0; i < elements_to_move; ++i) {
            size_type src_idx = index + count + i;
            size_type dst_idx = src_idx - count;

            m_buffer[dst_idx] = std::move_if_noexcept(m_buffer[src_idx]);
            ++moved;
        }

        guard.dismiss();
    }
}








template <typename T, typename S>
void vector<T, S>::m_grow(size_type increase)
{
    size_type required = m_required_capacity(increase);
    if (required <= m_capacity) return;

    size_type newCapacity = m_growth_capacity(required);
    T* ptr = m_alloc(newCapacity);

    if constexpr (m_is_trivial_type)
        std::memcpy(ptr, m_buffer, sizeof(T) * newCapacity);
    else
    {
        size_type constructed = 0;

        auto guard = jman::scope_guard{[&](){
            // destroys the constructed and deallocated and acts like nothing happened
            if constexpr (m_needs_rollback_on_construct) {
                jman::destruct_at(constructed, ptr);
                m_dealloc(ptr);
            }
        }};

        for (size_type i = 0; i < m_size; ++i) {
            jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
            ++constructed;
        }

        // discard the guard, nothing can throw past this point
        guard.dismiss();
    }
    jman::destruct_at(m_buffer, m_size);

    if (m_buffer) m_dealloc(m_buffer);

    m_buffer = ptr;
    m_capacity = newCapacity;
}





template <typename T, typename S>
template <bool Move>
void vector<T, S>::m_insert_range(size_type index, T* src, size_type count)
{
    if (count == 0) return;
    if (index > m_size) JMAN_THROW(std::out_of_range("jman::vector<T>::m_insert_range() failed: Index out of range."));
    size_type required = m_required_capacity(count);

    if (m_capacity >= required)
        // enough space to fit the elements specified
    {
        m_shift_right(index, count);

        for (size_type i = 0; i < count; ++i) {
            jman::destruct_at(m_buffer + index + i);
            if constexpr (Move)
                jman::construct_at<T>(m_buffer + index + i, std::move_if_noexcept(src[i]));
            else
                jman::construct_at<T>(m_buffer + index + i, src[i]);
        }
        m_size += count;
    } else
        // need to allocate a new block to fit the elements
    {
        size_type newCapacity = m_growth_capacity(required);
        size_type constructed = 0;

        T* newBuffer = m_alloc(newCapacity);

        auto guard = jman::scope_guard{[&](){
                if constexpr (
                    m_needs_rollback_on_assign || m_needs_rollback_on_construct
                ){
                    jman::destruct_at(constructed, newBuffer);
                    m_dealloc(newBuffer);
                }
            }};

        if constexpr (m_is_trivial_type) {
            // copy the elements before the insertion point
            std::memcpy(newBuffer, m_buffer, sizeof(T) * index);
            // copy the elements past the insertion point
            std::memcpy(newBuffer + index + count, m_buffer + index, sizeof(T) * (m_size - index));
            constructed = m_size;
        } else {

            // move/copy the elements before the insertion point
            for (size_type i = 0; i < index; ++i) {
                if constexpr (Move)
                    jman::construct_at<T>(newBuffer + i, std::move_if_noexcept(m_buffer[i]));
                else
                    jman::construct_at<T>(newBuffer + i, m_buffer[i]);
                ++constructed;
            }

            // move/copy the elements after the insertion points
            for (size_type i = index; i < m_size; ++i) {
                if constexpr (Move)
                    jman::construct_at<T>(newBuffer + i + count, std::move_if_noexcept(m_buffer[i]));
                else
                    jman::construct_at<T>(newBuffer + i + count, m_buffer[i]);
                ++constructed;
            }
        }

        // construct the element at the insertion point
        for (size_type i = 0; i < count; ++i) {
            if constexpr (Move)
                jman::construct_at<T>(newBuffer + index + i, std::move_if_noexcept(src[i]));
            else
                jman::construct_at<T>(newBuffer + index + i, src[i]);
            ++constructed;
        }

        // discard the guard, nothing past this line can throw
        guard.dismiss();

        jman::destruct_at(m_buffer, m_size);
        if (m_buffer) m_dealloc(m_buffer);

        m_buffer = newBuffer;
        m_capacity = newCapacity;
        m_size += count;
    }
}

// #####################################################################################################################
// -------------------------------------------- CONSTRUCTORS / DESTRUCTORS ------------------------------------------//
// #####################################################################################################################

// ---------------------------------------------- regular constructors

template <typename T, typename S>
vector<T, S>::vector(std::initializer_list<T> list)
{
    if (list.size() == 0) return;

    size_type newCapacity = jman::max(static_cast<size_type>(list.size()), MIN_CAPACITY);
    m_buffer = m_alloc(newCapacity);

    if (m_is_trivial_type) {
        std::memcpy(m_buffer, list.begin(), list.size() * sizeof(T));
        m_size = list.size();
    } else {
        auto guard = jman::scope_guard{[&](){
            if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
                jman::destruct_at(m_size, m_buffer);
                m_dealloc(m_buffer);

                m_buffer = nullptr;
                m_size = 0;
            }
        }};

        for (const T& value: list) {
            jman::construct_at<T>(m_buffer + m_size, value);
            ++m_size;
        }
        guard.dismiss();
    }

    m_capacity = newCapacity;
}

template <typename T, typename S>
vector<T, S>::vector(size_type size)
{
    if (size == 0) return;

    size_type newCapacity = jman::max(static_cast<size_type>(size), MIN_CAPACITY);
    m_buffer = m_alloc(newCapacity);

    auto guard = jman::scope_guard{[&](){
        if constexpr (!std::is_nothrow_default_constructible_v<T>) {
            jman::destruct_at(m_size, m_buffer);
            m_dealloc(m_buffer);

            m_buffer = nullptr;
            m_size = 0;
        }
    }};

    for (size_type i = 0; i < size; ++i) {
        jman::construct_at<T>(m_buffer + m_size);
        ++m_size;
    }

    guard.dismiss();
    m_capacity = newCapacity;
}

template <typename T, typename S>
vector<T, S>::vector(size_type size, const T& value)
{
    if (size == 0) return;

    size_type newCapacity = jman::max(static_cast<size_type>(size), MIN_CAPACITY);
    m_buffer = m_alloc(newCapacity);

    auto guard = jman::scope_guard{[&](){
        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            jman::destruct_at(m_size, m_buffer);
            m_dealloc(m_buffer);

            m_buffer = nullptr;
            m_size = 0;
        }
    }};

    for (size_type i = 0; i < size; ++i) {
        jman::construct_at<T>(m_buffer + i, value);
        ++m_size;
    }

    guard.dismiss();
    m_capacity = newCapacity;
}


// ---------------------------------------------- copy constructors

template <typename T, typename S>
vector<T, S>::vector(const vector& other)
{
    if (other.empty()) return;

    size_type newCapacity = other.m_capacity;
    m_buffer = m_alloc(newCapacity);

    if (m_is_trivial_type) {
        std::memcpy(m_buffer, other.m_buffer, other.m_size * sizeof(T));
        m_size = other.m_size;
    } else {
        auto guard = jman::scope_guard{[&](){
            if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
                jman::destruct_at(m_size, m_buffer);
                m_dealloc(m_buffer);

                m_buffer = nullptr;
                m_size = 0;
            }
        }};

        for (size_type i = 0; i < other.m_size; ++i) {
            jman::construct_at<T>(m_buffer + i, other.m_buffer[i]);
            ++m_size;
        }
        guard.dismiss();
    }

    m_capacity = newCapacity;
}

template <typename T, typename S>
vector<T, S>& vector<T, S>::operator=(const vector& other)
{
    if (other.empty()) return *this;
    if (this == &other) return *this;

    // if the buffer was allocated to fit the new vector's elements, just delete it and the old buffer remains intact
    // but if the buffer is reused, dont do anything and the buffer will remain in a mid/weird state

    if (m_capacity < other.m_size)
        // allocate a new block since the current vector cant hold the other vector
    {
        T* buff = m_alloc(other.m_capacity);

        if (m_is_trivial_type)
            std::memcpy(buff, other.m_buffer, sizeof(T) * other.m_size);
        else {
            size_type constructed = 0;

            auto guard = jman::scope_guard{[&](){
                if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
                    jman::destruct_at(constructed, buff);
                    m_dealloc(buff);
                }
            }};

            for (size_type i = 0; i < other.m_size; ++i) {
                jman::construct_at<T>(buff + i, other.m_buffer[i]);
                ++constructed;
            }
            guard.dismiss();
        }

        clear();
        if (m_buffer) m_dealloc(m_buffer);

        m_buffer = buff;
        m_capacity = other.m_capacity;
        m_size = other.m_size;
    } else
        // safely reuse the current block
    {
        if (m_is_trivial_type)
            std::memcpy(m_buffer, other.m_buffer, sizeof(T) * other.m_size);
        else
            for (size_type i = 0; i < other.m_size; ++i)
                m_buffer[i] = other.m_buffer[i];

        jman::destruct_at(m_buffer + other.m_size, m_capacity - other.m_size);

        m_size = other.m_size;
    }

    return *this;
}


// ---------------------------------------------- move constructors

template <typename T, typename S>
vector<T, S>::vector(vector&& other) noexcept
{
    m_buffer = other.m_buffer;
    m_capacity = other.m_capacity;
    m_size = other.m_size;

    other.m_buffer = nullptr;
    other.m_capacity = 0;
    other.m_size = 0;
}

template <typename T, typename S>
vector<T, S>& vector<T, S>::operator=(vector&& other) noexcept
{
    if (this != &other) return *this;

    clear();
    if (m_buffer) m_dealloc(m_buffer);

    m_buffer = other.m_buffer;
    m_capacity = other.m_capacity;
    m_size = other.m_size;

    other.m_buffer = nullptr;
    other.m_capacity = 0;
    other.m_size = 0;

    return *this;
}

template <typename T, typename S>
vector<T, S>& vector<T, S>::operator=(std::initializer_list<T> list)
{
    if (list.size() == 0) return *this;

    // if the buffer was allocated to fit the new vector's elements, just delete it and the old buffer remains intact
    // but if the buffer is reused, dont do anything and the buffer will remain in a mid/weird state

    size_type newCapacity = jman::max(static_cast<size_type>(list.size()), MIN_CAPACITY);

    if (m_capacity < newCapacity)
        // allocate a new block since the current vector cant hold the other vector
    {
        T* buff = m_alloc(newCapacity);

        if (m_is_trivial_type)
            std::memcpy(buff, list.begin(), sizeof(T) * list.size());
        else {
            size_type constructed = 0;

            auto guard = jman::scope_guard{[&](){
                if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
                    jman::destruct_at(constructed, buff);
                    m_dealloc(buff);
                }
            }};


            for (size_type i = 0; i < list.size(); ++i) {
                jman::construct_at<T>(buff + i, list[i]);
                ++constructed;
            }
            guard.dismiss();
        }


        clear();
        if (m_buffer) m_dealloc(m_buffer);

        m_buffer = buff;
        m_capacity = newCapacity;
        m_size = list.size();
    } else
        // safely reuse the current block
    {
        if (m_is_trivial_type)
            std::memcpy(m_buffer, list.begin(), sizeof(T) * list.size());
        else
            for (size_type i = 0; i < list.size(); ++i)
                m_buffer[i] = list[i];

        jman::destruct_at(m_buffer + list.size(), m_capacity - list.size());

        m_size = list.size();
    }

    return *this;
}


// ---------------------------------------------- destructor

template <typename T, typename S>
vector<T, S>::~vector() noexcept
{
    reset();
}


// ##########################################################################################################
// ----------------------------------------------- ACCESSING ----------------------------------------------//
// ##########################################################################################################

template <typename T, typename S>
JMAN_NO_DISCARD JMAN_FORCE_INLINE vector<T, S>::reference vector<T, S>::front_or(reference fallback) noexcept
{
    if (empty()) return fallback;
    return m_buffer[0];
}
template <typename T, typename S>
JMAN_NO_DISCARD JMAN_FORCE_INLINE vector<T, S>::reference vector<T, S>::back_or(reference fallback) noexcept
{
    if (empty()) return fallback;
    return m_buffer[m_size - 1];
}

template <typename T, typename S>
JMAN_NO_DISCARD JMAN_FORCE_INLINE vector<T, S>::const_reference vector<T, S>::front_or(reference fallback) const noexcept
{
    if (empty()) return fallback;
    return m_buffer[0];
}
template <typename T, typename S>
JMAN_NO_DISCARD JMAN_FORCE_INLINE vector<T, S>::const_reference vector<T, S>::back_or(reference fallback) const noexcept
{
    if (empty()) return fallback;
    return m_buffer[m_size - 1];
}





template <typename T, typename S>
JMAN_NO_DISCARD vector<T, S>::reference vector<T, S>::at(size_type index)
{
    if (empty() || index >= m_size)
        JMAN_THROW(std::out_of_range("jman::vector<T, S>::at() failed: Out of bounds when trying to index an object or vector is empty."));
    return m_buffer[index];
}

template <typename T, typename S>
JMAN_NO_DISCARD vector<T, S>::const_reference vector<T, S>::at(size_type index) const
{
    if (empty() || index >= m_size)
        JMAN_THROW(std::out_of_range("jman::vector<T, S>::at() failed: Out of bounds when trying to index an object or vector is empty."));
    return m_buffer[index];
}





template <typename T, typename S>
JMAN_NO_DISCARD JMAN_FORCE_INLINE vector<T, S>::iterator vector<T, S>::find(size_type index) noexcept
{
    if (empty() || index >= m_size) return end();
    return iterator{m_buffer + index};
}

template <typename T, typename S>
JMAN_NO_DISCARD JMAN_FORCE_INLINE vector<T, S>::const_iterator vector<T, S>::find(size_type index) const noexcept
{
    if (empty() || index >= m_size) return cend();
    return const_iterator{m_buffer + index};
}






template <typename T, typename S>
JMAN_NO_DISCARD bool vector<T, S>::contains(const_reference value) const
    noexcept(noexcept(std::declval<T>() == std::declval<T>()))
{
    if (empty()) return false;

    for (size_type i = 0; i < m_size; ++i)
            if (m_buffer[i] == value) return true;

    return false;
}

// ##########################################################################################################
// -------------------------------------------- MEMORY MODIFIERS ------------------------------------------//
// ##########################################################################################################

template <typename T, typename S>
void vector<T, S>::reserve(size_type count)
{
    if (count <= m_capacity) return;

    T* reserved = m_alloc(count);

    if (m_is_trivial_type)
        std::memcpy(reserved, m_buffer, sizeof(T) * m_size);
    else
    {
        size_type constructed = 0;

        auto guard = jman::scope_guard{[&](){
            if constexpr (m_needs_rollback_on_construct) {
                jman::destruct_at(reserved, constructed);
                m_dealloc(reserved);
            }
        }};

        for (size_type i = 0; i < m_size; ++i) {
            jman::construct_at<T>(reserved + i, std::move_if_noexcept(m_buffer[i]));
            ++constructed;
        }

        guard.dismiss();
    }

    jman::destruct_at(m_buffer, m_size);
    if (m_buffer) m_dealloc(m_buffer);

    m_buffer = reserved;
    m_capacity = count;
}

template <typename T, typename S>
void vector<T, S>::set_capacity(size_type capacity)
{
    if (capacity > m_capacity)
    // increase the capacity of the vector
        reserve(capacity);
    else if (capacity < m_capacity)
    // truncate the capacity of the vector and its elements
        resize(capacity);
}




template <typename T, typename S>
void vector<T, S>::clear() noexcept
{
    jman::destruct_at(m_buffer, m_size);
    m_size = 0;
}

template <typename T, typename S>
void vector<T, S>::reset() noexcept
{
    clear();
    if (m_buffer) m_dealloc(m_buffer);
    m_buffer = nullptr;
    m_capacity = 0;
}





template <typename T, typename S>
void vector<T, S>::resize(size_type newSize)
{
    if (newSize > m_size)
    // increase the capacity of the vector and replace the empty gaps with default constructed elements
    {
        T* ptr = m_alloc(newSize);
        size_type constructed = 0;

        auto guard = jman::scope_guard{[&](){
            if constexpr (
                m_needs_rollback_on_construct &&
                !std::is_nothrow_default_constructible_v<T>
            ) {
                jman::destruct_at(constructed, ptr);
                m_dealloc(ptr);
            }
        }};

        if (m_is_trivial_type) {
            std::memcpy(ptr, m_buffer, sizeof(T) * m_size);
            constructed = m_size;
        } else
            for (size_type i = 0; i < m_size; ++i) {
                jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
                ++constructed;
            }

        for (size_type i = m_size; i < newSize; ++i) {
            jman::construct_at<T>(ptr + i);
            ++constructed;
        }

        guard.dismiss();

        jman::destruct_at(m_buffer, m_size);
        if (m_buffer) m_dealloc(m_buffer);

        m_buffer = ptr;
        m_size = m_capacity = newSize;
    }
    else if (newSize < m_size)
    // decrease the contents and capacity of the vector
    {
        jman::destruct_at(m_size - newSize, m_buffer + m_size - 1);
        m_size = newSize;

        if (newSize == 0) {
            if (m_buffer) m_dealloc(m_buffer);
            m_buffer = nullptr;
            m_capacity = 0;
            return;
        }

        T* ptr = m_alloc(newSize);

        if (m_is_trivial_type)
            std::memcpy(ptr, m_buffer, sizeof(T) * newSize);
        else
        {
            size_type constructed = 0;

            auto guard = jman::scope_guard{[&](){
                if constexpr (m_needs_rollback_on_construct) {
                    jman::destruct_at(ptr, constructed);
                    m_dealloc(ptr);
                }
            }};

            for (size_type i = 0; i < newSize; ++i) {
                jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
                ++constructed;
            }

            guard.dismiss();
        }

        jman::destruct_at(m_buffer, m_size);

        if (m_buffer) m_dealloc(m_buffer);

        m_buffer = ptr;
        m_capacity = newSize;
    }
}

template <typename T, typename S>
void vector<T, S>::resize(S newSize, const_reference value)
{
    if (newSize > m_size)
    // increase the capacity of the vector and replace the empty gaps with default constructed elements
    {
        T* ptr = m_alloc(newSize);
        size_type constructed = 0;

        auto guard = jman::scope_guard{[&](){
            if constexpr (
                m_needs_rollback_on_construct &&
                !std::is_nothrow_default_constructible_v<T>
            ) {
                jman::destruct_at(constructed, ptr);
                m_dealloc(ptr);
            }
        }};

        if (m_is_trivial_type) {
            std::memcpy(ptr, m_buffer, sizeof(T) * m_size);
            constructed = m_size;
        } else
            for (size_type i = 0; i < m_size; ++i) {
                jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
                ++constructed;
            }

        for (size_type i = m_size; i < newSize; ++i) {
            jman::construct_at<T>(ptr + i, value);
            ++constructed;
        }

        guard.dismiss();

        jman::destruct_at(m_buffer, m_size);
        if (m_buffer) m_dealloc(m_buffer);

        m_buffer = ptr;
        m_size = m_capacity = newSize;
    }
    else if (newSize < m_size)
    // decrease the contents and capacity of the vector
    {
        jman::destruct_at(m_size - newSize, m_buffer + m_size - 1);
        m_size = newSize;

        if (newSize == 0) {
            if (m_buffer) m_dealloc(m_buffer);
            m_buffer = nullptr;
            m_capacity = 0;
            return;
        }

        T* ptr = m_alloc(newSize);

        if (m_is_trivial_type)
            std::memcpy(ptr, m_buffer, sizeof(T) * newSize);
        else
        {
            size_type constructed = 0;

            auto guard = jman::scope_guard{[&](){
                if constexpr (m_needs_rollback_on_construct) {
                    jman::destruct_at(ptr, constructed);
                    m_dealloc(ptr);
                }
            }};

            for (size_type i = 0; i < newSize; ++i) {
                jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
                ++constructed;
            }

            guard.dismiss();
        }

        jman::destruct_at(m_buffer, m_size);

        if (m_buffer) m_dealloc(m_buffer);

        m_buffer = ptr;
        m_capacity = newSize;
    }
}





template <typename T, typename S>
void vector<T, S>::shrink_to_fit()
{
    if (m_size == m_capacity) return;
    resize(m_size);
}

template <typename T, typename S>
void vector<T, S>::swap(vector& other) noexcept
{
    size_type capacity = m_capacity,
              size = m_size;
    T* buff = m_buffer;

    m_buffer = other.m_buffer;
    m_capacity = other.m_capacity;
    m_size = other.m_size;

    other.m_buffer = buff;
    other.m_size = size;
    other.m_capacity = capacity;
}


// ##########################################################################################################
// ---------------------------------------------- ALGORITHMS ----------------------------------------------//
// ##########################################################################################################

template <typename T, typename S>
template <std::predicate<T> Predicate>
JMAN_NO_DISCARD JMAN_FORCE_INLINE vector<T, S>::iterator vector<T, S>::find_first_if(Predicate pred)
    noexcept(std::is_nothrow_invocable_r_v<bool, Predicate, const T&>)
{
    for (size_type i = 0; i < m_size; ++i)
        if (pred(m_buffer[i])) return iterator{m_buffer + i};
    return end();
}

template <typename T, typename S>
template <std::predicate<T> Predicate>
JMAN_NO_DISCARD JMAN_FORCE_INLINE vector<T, S>::const_iterator vector<T, S>::find_first_if(Predicate pred) const
    noexcept(std::is_nothrow_invocable_r_v<bool, Predicate, const T&>)
{
    for (size_type i = 0; i < m_size; ++i)
        if (pred(m_buffer[i])) return const_iterator{m_buffer + i};
    return cend();
}



template <typename T, typename S>
JMAN_NO_DISCARD JMAN_FORCE_INLINE vector<T, S>::iterator vector<T, S>::find_first(const_reference value)
    noexcept(noexcept(std::declval<T>() == std::declval<T>()))
{
    for (size_type i = 0; i < m_size; ++i)
        if (m_buffer[i] == value) return iterator{m_buffer + i};
    return end();
}

template <typename T, typename S>
JMAN_NO_DISCARD JMAN_FORCE_INLINE vector<T, S>::const_iterator vector<T, S>::find_first(const_reference value) const
    noexcept(noexcept(std::declval<T>() == std::declval<T>()))
{
    for (size_type i = 0; i < m_size; ++i)
        if (m_buffer[i] == value) return const_iterator{m_buffer + i};
    return cend();
}





template <typename T, typename S>
template <std::predicate<T> Predicate>
void vector<T, S>::remove_if(Predicate pred)
    noexcept(!m_needs_rollback_on_assign && std::is_nothrow_invocable_r_v<bool, Predicate, const T&>)
{
    if (empty()) return;

    auto dest = find_first_if(pred);
    if (dest == end()) return;

    auto original_end = end();

    for (auto i = dest; i != end(); ++i) {
        // if *i should stay, move it to the next valid index
        if (!pred(*i))
            *dest++ = std::move_if_noexcept(*i);
    }

    // destroy the remaining objects
    jman::destruct_at(jman::addressof(*dest), static_cast<size_type>(original_end - dest));
    m_size = dest - begin();
}

template <typename T, typename S>
void vector<T, S>::remove(const_reference value)
    noexcept(!m_needs_rollback_on_assign && noexcept(std::declval<T>() == std::declval<T>()))
{
    if (empty()) return;

    auto dest = find_first(value);
    if (dest == end()) return;

    auto original_end = end();

    for (auto i = dest; i != end(); ++i)
        // if *i is not the value, move it to the next valid index
        if (*i != value)
            *dest++ = std::move_if_noexcept(*i);

    // destroy the remaining objects
    jman::destruct_at(jman::addressof(*dest), static_cast<size_type>(original_end - dest));
    m_size = dest - begin();
}



template <typename T, typename S>
void vector<T, S>::reverse()
    noexcept(std::is_nothrow_swappable_v<T>)
{
    if (empty()) return;

    T* l = m_buffer + 0;
    T* r = m_buffer + m_size - 1;
    size_type moved = 0;

    auto guard = jman::scope_guard{[&](){
        if constexpr (!std::is_nothrow_swappable_v<T>) {
            while (moved > 0) {
                --l;
                ++r;
                --moved;
                std::swap(*l, *r);
            }
        }
    }};

    while (l < r) {
        std::swap(*l, *r);
        ++l;
        --r;
        ++moved;
    }
}


template <typename T, typename S>
vector<T, S>::size_type vector<T, S>::unique()
    noexcept(!m_needs_rollback_on_assign && noexcept(std::declval<T>() == std::declval<T>()))
{
    if (empty()) return 0;

    auto result = begin();
    auto first = begin();
    auto original_end = end();

    size_type removed = 0;

    while (++first != end())
        if (*result != *first && ++result != first)
            *result = std::move_if_noexcept(*first);
        else
            ++removed;

    ++result;
    jman::destruct_at(jman::addressof(*result), static_cast<size_type>(original_end - result));

    m_size = result - begin();
    return removed;
}
template <typename T, typename S>
template <std::predicate<T, T> Predicate>
vector<T, S>::size_type vector<T, S>::unique(Predicate pred)
    noexcept(!m_needs_rollback_on_assign && std::is_nothrow_invocable_r_v<bool, Predicate, const T&>)
{
    if (empty()) return 0;

    auto result = begin();
    auto first = begin();
    auto original_end = end();

    size_type removed = 0;

    while (++first != end())
        if (!pred(*result, *first) && ++result != first)
            *result = std::move_if_noexcept(*first);
        else
            ++removed;

    ++result;
    jman::destruct_at(jman::addressof(*result), static_cast<size_type>(original_end - result));

    m_size = result - begin();
    return removed;
}



template <typename T, typename S>
template <std::predicate<T> Predicate>
void vector<T, S>::retain(Predicate pred)
    noexcept(!m_needs_rollback_on_assign && std::is_nothrow_invocable_r_v<bool, Predicate, const T&>)
{
    if (empty()) return;

    auto dest = find_first_if(pred);
    if (dest == end()) return;

    auto original_end = end();

    for (auto i = ++dest; i != end(); ++i) {
         // if predicate is true move the item to the next valid position
        if (pred(*i))
            *dest++ = std::move_if_noexcept(*i);
    }

    // destroy the remaining elements
    jman::destruct_at(jman::addressof(*dest), static_cast<size_type>(original_end - dest));
    m_size = dest - begin();
}


// ##########################################################################################################
// ------------------------------------------ ELEMENTS INSERTION ------------------------------------------//
// ##########################################################################################################

template <typename T, typename S>
template <typename ...Args> requires std::is_constructible_v<T, Args...>
void vector<T, S>::emplace_back(Args&&... args)
{
    m_grow();
    jman::construct_at<T>(m_buffer + m_size, std::forward<Args>(args)...);
    ++m_size;
}
template <typename T, typename S>
template <typename TT> requires std::is_convertible_v<TT, T>
void vector<T, S>::push_back(TT&& value)
{
    m_grow();
    jman::construct_at<T>(m_buffer + m_size, static_cast<T>(std::forward<TT>(value)));
    ++m_size;
}




template <typename T, typename S>
template <typename TT> requires std::is_convertible_v<TT, T>
void vector<T, S>::insert(size_type index, TT&& value)
{
    if (index > m_size) JMAN_THROW(std::out_of_range("jman::vector<T>::insert() failed: Index out of range."));
    size_type required = m_required_capacity(1);

    if (m_capacity >= required)
        // enough space to fit one more element
    {
        m_shift_right(index, 1);

        jman::destruct_at(m_buffer + index);
        jman::construct_at<T>(m_buffer + index, static_cast<T>(std::forward<TT>(value)));
        ++m_size;
    } else
        // no space available, allocate a new block
    {
        size_type newCapacity = m_growth_capacity(required);

        T* newBuffer = m_alloc(newCapacity);
        size_type constructed = 0;

        auto guard = jman::scope_guard{[&](){
            if constexpr (
                m_needs_rollback_on_assign ||
                m_needs_rollback_on_construct
            ){
                jman::destruct_at(constructed, newBuffer);
                m_dealloc(newBuffer);
            }
        }};

        if (m_is_trivial_type) {
            // copy the elements before the insertion point
            std::memcpy(newBuffer, m_buffer, sizeof(T) * index);
            // copy the elements past the insertion point
            std::memcpy(newBuffer + index + 1, m_buffer + index, sizeof(T) * (m_size - index));
            constructed = m_size;
        } else {
            // move/copy the elements before the insertion point
            for (size_type i = 0; i < index; ++i) {
                jman::construct_at<T>(newBuffer + i, std::move_if_noexcept(m_buffer[i]));
                ++constructed;
            }

            // move/copy the elements after the insertion points
            for (size_type i = index; i < m_size; ++i) {
                jman::construct_at<T>(newBuffer + i + 1, std::move_if_noexcept(m_buffer[i]));
                ++constructed;
            }
        }

        // construct the element at the insertion point
        jman::construct_at<T>(newBuffer + index, static_cast<T>(std::forward<TT>(value)));
        ++constructed;

        // discard the guard, nothing past this line can throw
        guard.dismiss();

        jman::destruct_at(m_buffer, m_size);
        if (m_buffer) m_dealloc(m_buffer);

        m_buffer = newBuffer;
        m_capacity = newCapacity;
        ++m_size;
    }
}
template <typename T, typename S>
void vector<T, S>::insert(size_type index, size_type count, const_reference value)
{
    if (count == 0) return;
    if (index > m_size) JMAN_THROW(std::out_of_range("jman::vector<T>::insert() failed: Index out of range."));
    size_type required = m_required_capacity(count);

    if (m_capacity >= required)
        // enough space to fit the elements specified
    {
        m_shift_right(index, count);

        for (size_type i = 0; i < count; ++i) {
            jman::destruct_at(m_buffer + index + i);
            jman::construct_at<T>(m_buffer + index + i, value);
        }
        m_size += count;
    } else
        // need to allocate a new block to fit the elements
    {
        size_type newCapacity = m_growth_capacity(required);

        T* newBuffer = m_alloc(newCapacity);
        size_type constructed = 0;

        auto guard = jman::scope_guard{[&](){
            if constexpr (
                m_needs_rollback_on_assign ||
                m_needs_rollback_on_construct
            ){
                jman::destruct_at(constructed, newBuffer);
                m_dealloc(newBuffer);
            }
        }};
        if (m_is_trivial_type) {
            // copy the elements before the insertion point
            std::memcpy(newBuffer, m_buffer, sizeof(T) * index);
            // copy the elements past the insertion point
            std::memcpy(newBuffer + index + count, m_buffer + index, sizeof(T) * (m_size - index));
            constructed = m_size;
        } else {
            // move/copy the elements before the insertion point
            for (size_type i = 0; i < index; ++i) {
                jman::construct_at<T>(newBuffer + i, std::move_if_noexcept(m_buffer[i]));
                ++constructed;
            }
            // move/copy the elements after the insertion points
            for (size_type i = index; i < m_size; ++i) {
                jman::construct_at<T>(newBuffer + i + count, std::move_if_noexcept(m_buffer[i]));
                ++constructed;
            }
        }

        // construct the element at the insertion point
        for (size_type i = 0; i < count; ++i) {
            jman::construct_at<T>(newBuffer + index + i, value);
            ++constructed;
        }

        // discard the guard, nothing past this line can throw
        guard.dismiss();

        jman::destruct_at(m_buffer, m_size);
        if (m_buffer) m_dealloc(m_buffer);

        m_buffer = newBuffer;
        m_capacity = newCapacity;
        m_size += count;
    }
}
template <typename T, typename S>
void vector<T, S>::insert(size_type index, std::initializer_list<T> list)
{
    if (list.size() == 0) return;
    m_insert_range<false>(index, list.begin(), list.size());
}



template <typename T, typename S>
template <typename ...Args> requires std::is_constructible_v<T, Args...>
void vector<T, S>::emplace(size_type index, Args&&... args)
{
    if (index > m_size) JMAN_THROW(std::out_of_range("jman::vector<T>::emplace() failed: Index out of range."));
    insert(index, T{std::forward<Args>(args)...});
}




template <typename T, typename S>
template <typename TT> requires std::is_convertible_v<TT, T>
vector<T, S>::iterator vector<T, S>::insert(const_iterator it, TT&& value)
{
    size_type index = it - begin();
    insert(index, std::forward<TT>(value));
    return iterator{m_buffer + index};
}
template <typename T, typename S>
vector<T, S>::iterator vector<T, S>::insert(const_iterator it, size_type count, const_reference value)
{
    size_type index = it - begin();
    insert(index, count, value);
    return iterator{m_buffer + index};
}
template <typename T, typename S>
vector<T, S>::iterator vector<T, S>::insert(const_iterator it, std::initializer_list<T> list)
{
    size_type index = it - begin();
    if (list.size() == 0) return iterator{m_buffer + index};

    m_insert_range<false>(index, list.begin(), list.size());

    return iterator{m_buffer + index};
}




template <typename T, typename S>
template <typename ...Args>
vector<T, S>::iterator vector<T, S>::emplace(const_iterator it, Args&&... args)
{
    size_type index = it - begin();
    if (index > m_size) JMAN_THROW(std::out_of_range("jman::vector<T>::emplace() failed: Index out of range."));

    insert(index, T{std::forward<Args>(args)...});
    return iterator{m_buffer + index};
}




template <typename T, typename S>
void vector<T, S>::assign(size_type count, const T& value)
{
    if (count == m_capacity) {
        size_type oldSize = m_size;
        m_size = 0;

        for (size_type i = 0; i < count; ++i) {
            if (i < oldSize)
                jman::destruct_at(m_buffer + i);

            jman::construct_at<T>(m_buffer + i, value);
            ++m_size;
        }

        m_capacity = m_size = count;
    } else {
        reset();
        resize(count, value);
    }
}





template <typename T, typename S>
vector<T, S>::iterator vector<T, S>::append(const vector& vec)
{
    if (vec.empty()) return end();

    size_type insert_point = m_size;
    m_insert_range<false>(insert_point, vec.m_buffer, vec.size());

    return iterator{m_buffer + insert_point};
}
template <typename T, typename S>
vector<T, S>::iterator vector<T, S>::append(vector&& vec)
{
    if (vec.empty()) return end();

    size_type insert_point = m_size;
    m_insert_range<true>(m_size, vec.m_buffer, vec.size());
    vec.reset();

    return iterator{m_buffer + insert_point};
}

template <typename T, typename S>
void vector<T, S>::append(size_type index, const vector& vec)
{
    if (vec.empty()) return;
    m_insert_range<false>(index, vec.m_buffer, vec.size());
}
template <typename T, typename S>
void vector<T, S>::append(size_type index, vector&& vec)
{
    if (vec.empty()) return;

    m_insert_range<true>(index, vec.m_buffer, vec.size());
    vec.reset();
}

template <typename T, typename S>
vector<T, S>::iterator vector<T, S>::append(const_iterator it, const vector& vec)
{
    size_type index = it - begin();
    if (vec.empty()) return iterator{m_buffer + index};

    m_insert_range<false>(index, vec.m_buffer, vec.size());
    return iterator{m_buffer + index};
}
template <typename T, typename S>
vector<T, S>::iterator vector<T, S>::append(const_iterator it, vector&& vec)
{
    size_type index = it - begin();
    if (vec.empty()) return iterator{m_buffer + index};

    m_insert_range<true>(index, vec.m_buffer, vec.size());
    vec.reset();

    return iterator{m_buffer + index};
}


// ##########################################################################################################
// ------------------------------------------- ELEMENTS REMOVAL -------------------------------------------//
// ##########################################################################################################

template <typename T, typename S>
void vector<T, S>::pop_back() noexcept
{
    if (empty()) return;
    --m_size;
    jman::destruct_at(m_buffer + m_size);
}


template <typename T, typename S>
vector<T, S>::size_type vector<T, S>::erase(size_type index)
{
    if (index >= m_size) JMAN_THROW(std::out_of_range("jman::vector<T>::erase() failed: Index out of range."));
    if (m_size == 1) {
        reset();
        return 0;
    }
    m_shift_left(index, 1);

    --m_size;
    jman::destruct_at(m_buffer + m_size);
    return m_size;
}
template <typename T, typename S>
vector<T, S>::size_type vector<T, S>::erase(size_type first, size_type last)
{
    if (first >= m_size) JMAN_THROW(std::out_of_range("jman::vector<T>::erase() failed: Index 'first' out of range."));
    if (last > m_size) JMAN_THROW(std::out_of_range("jman::vector<T>::erase() failed: Index 'last' out of range."));

    size_type count = last - first;

    if (count == 0)
        return last;
    else if (count == m_size) {
        reset();
        return 0;
    }

    m_shift_left(first, count);
    jman::destruct_at(m_buffer + m_size - count, count);

    m_size -= count;
    return m_size;
}


template <typename T, typename S>
vector<T, S>::iterator vector<T, S>::erase(const_iterator it)
{
    size_type index = it - begin();
    return iterator{m_buffer + erase(index)};
}
template <typename T, typename S>
vector<T, S>::iterator vector<T, S>::erase(const_iterator first, const_iterator last)
{
    size_type firstIndex = first - begin();
    size_type lastIndex = last - begin();
    return iterator{m_buffer + erase(firstIndex, lastIndex)};
}


} // namespace jman