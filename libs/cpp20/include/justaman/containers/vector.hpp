#pragma once

#include "justaman/errors/safe_exceptions.hpp"
#include <justaman/memory/lifetime.hpp>
#include <justaman/utils/attributes.h>
#include <justaman/algorithms/numeric.hpp>
#include <justaman/utils/scope.hpp>
#include <iterator>
#include <limits>
#include <new>
#include <span>
#include <type_traits>
#include <stdexcept>

namespace jman 
{
    /**
        @brief 
            My custom implementation for a simpler version for std::vector.
            \n
            tried to make it as accuarate as the standard version while giving more flexibility but providing the same or even more performance and security
        \n
        Iterators:
            iterators work the same as std::vector and have the same rules
        
    */
    template <typename T, std::unsigned_integral S = std::size_t>
    class vector  
    {
        static_assert(!std::is_reference_v<T>, "Type T can't be a reference of any kind.");
        static_assert(!std::is_void_v<T>, "Type T can't be a of void type.");
        static_assert(std::is_nothrow_destructible_v<T>, "Type T must has it's destructor marked as noexcept for safety.");
        
        public: 
            using value_type            = T;
            using size_type             = S;
            using difference_type       = std::ptrdiff_t;
            using reference             = T&;
            using const_reference       = const T&;
            using pointer               = T*;
            using const_pointer         = const T*;

            constexpr static size_type MIN_CAPACITY     = 16;
            constexpr static size_type GROW_FACTOR      = 2;

        private:
            T*          m_buffer        = nullptr;

            size_type   m_capacity      = 0;
            size_type   m_size          = 0;

        private:
            void __grow(size_type increase = 1);

        public:
            struct const_iterator 
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

                    const_iterator() noexcept = default;
                    explicit const_iterator(T* ptr) noexcept: m_ptr(ptr) {}

                    JMAN_FORCE_INLINE reference operator[](difference_type index) const { return m_ptr[index]; }

                    JMAN_FORCE_INLINE const_iterator& operator++() noexcept { m_ptr += 1; return *this; }
                    JMAN_FORCE_INLINE const_iterator operator++(int) noexcept { iterator retval = *this; ++(*this); return retval; }
                    JMAN_FORCE_INLINE const_iterator& operator--() noexcept { m_ptr -= 1; return *this; }
                    JMAN_FORCE_INLINE const_iterator operator--(int) noexcept { iterator retval = *this; --(*this); return retval; }
                    
                    JMAN_FORCE_INLINE const_iterator operator+(difference_type index) const noexcept { return const_iterator(m_ptr + index); }
                    JMAN_FORCE_INLINE const_iterator operator-(difference_type index) const noexcept { return const_iterator(m_ptr - index); }
                    
                    JMAN_FORCE_INLINE difference_type operator-(const const_iterator& other) const noexcept { return m_ptr - other.m_ptr; }
                    
                    JMAN_FORCE_INLINE const_iterator& operator+=(difference_type diff) noexcept { m_ptr += diff; return *this; }
                    JMAN_FORCE_INLINE const_iterator& operator-=(difference_type diff) noexcept { m_ptr -= diff; return *this; }
                    friend JMAN_FORCE_INLINE const_iterator operator+(difference_type diff, const_iterator it) noexcept { return {diff + it.m_ptr}; }

                    JMAN_FORCE_INLINE pointer operator->() const { return m_ptr; }
                    JMAN_FORCE_INLINE reference operator*() const { return *m_ptr; }

                    auto operator <=>(const const_iterator&) const = default;
            };

            struct iterator 
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

                    iterator() noexcept = default;
                    explicit iterator(T* ptr) noexcept: m_ptr(ptr) {}
                    
                    JMAN_FORCE_INLINE iterator& operator++() noexcept { ++m_ptr; return *this; }
                    JMAN_FORCE_INLINE iterator operator++(int) noexcept { iterator retval = *this; ++(*this); return retval; }
                    JMAN_FORCE_INLINE iterator& operator--() noexcept { --m_ptr; return *this; }
                    JMAN_FORCE_INLINE iterator operator--(int) noexcept { iterator retval = *this; --(*this); return retval; }

                    JMAN_FORCE_INLINE reference operator[](difference_type index) const { return m_ptr[index]; }
                    
                    JMAN_FORCE_INLINE iterator operator+(difference_type index) const noexcept { return iterator(m_ptr + index); }
                    JMAN_FORCE_INLINE iterator operator-(difference_type index) const noexcept { return iterator(m_ptr - index); }
                    
                    JMAN_FORCE_INLINE difference_type operator-(const iterator& other) const noexcept { return m_ptr - other.m_ptr; }
                    
                    JMAN_FORCE_INLINE iterator& operator+=(difference_type diff) noexcept { m_ptr += diff; return *this; }
                    JMAN_FORCE_INLINE iterator& operator-=(difference_type diff) noexcept { m_ptr -= diff; return *this; }
                    friend JMAN_FORCE_INLINE iterator operator+(difference_type diff, iterator it) noexcept { return {diff + it.m_ptr}; }

                    JMAN_FORCE_INLINE pointer operator->() const { return m_ptr; }
                    JMAN_FORCE_INLINE reference operator*() const { return *m_ptr; }

                    auto operator <=>(const iterator&) const = default;
                    operator const_iterator() const { return {m_ptr}; };
            };

            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            // --------------------------------- constructors

            vector() noexcept = default;
            vector(std::initializer_list<T> list);
            explicit vector(size_type size);
            vector(size_type size, const_reference value);

            vector(const vector& other);
            vector& operator=(const vector& other);

            vector(vector&& other) noexcept;
            vector& operator=(vector&& other) noexcept;

            ~vector() noexcept;

            // -------------------------------- stats

            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type                 capacity() const noexcept { return m_capacity; }
            /** @warning returned value is an aproximation. */
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type                 max_size() const noexcept { return std::numeric_limits<size_type>::max() / sizeof(T); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type                 size() const noexcept { return m_size; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool                      empty() const noexcept { return m_size == 0; }

            // ------------------------------ accessing

            JMAN_NO_DISCARD JMAN_FORCE_INLINE pointer                   data() noexcept { return std::launder(m_buffer); }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference                 front() noexcept { return m_buffer[0]; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference                 back() noexcept { return m_buffer[m_size - 1]; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference                 front_or(reference _default) noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference                 back_or(reference _default) noexcept;
            
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference                 operator[](size_type index) noexcept { return m_buffer[index]; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference                 at(size_type index);
            JMAN_NO_DISCARD JMAN_FORCE_INLINE pointer                   find(size_type index) noexcept;



            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_pointer             data() const noexcept { return std::launder(m_buffer); }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference           front() const noexcept { return m_buffer[0]; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference           back() const noexcept { return m_buffer[m_size - 1]; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference           front_or(reference _default) const noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference           back_or(reference _default) const noexcept;

            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference           operator[](size_type index) const noexcept { return m_buffer[index]; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference           at(size_type index) const;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_pointer             find(size_type index) const noexcept;

            bool                                                        contains(const_reference) const;

            std::span<T>                                                slice(size_t offset, size_t count);
            std::span<T>                                                left(size_t count);
            std::span<T>                                                right(size_t count);

             // ---------------------------- iterators

            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator                  begin() noexcept { return iterator{m_buffer}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator                  end() noexcept { return iterator{m_buffer + m_size}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator            begin() const noexcept { return const_iterator{m_buffer}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator            end() const noexcept { return const_iterator{m_buffer + m_size}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator            cbegin() const noexcept { return const_iterator {m_buffer}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator            cend() const noexcept { return const_iterator{m_buffer + m_size}; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE reverse_iterator          rbegin() noexcept { return reverse_iterator{iterator{m_buffer}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reverse_iterator          rend() noexcept { return reverse_iterator{iterator{m_buffer + m_size}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    rbegin() const noexcept { return const_reverse_iterator{const_iterator{m_buffer}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    rend() const noexcept { return const_reverse_iterator{const_iterator{m_buffer + m_size}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    crbegin() const noexcept { return const_reverse_iterator{const_iterator {m_buffer}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    crend() const noexcept { return const_reverse_iterator{const_iterator{m_buffer + m_size}}; }

            // -------------------------------- memory modifiers

            void                                                reserve(size_type count);
            void                                                clear() noexcept;
            /** @brief clears elements and frees the heap memory */
            void                                                reset() noexcept;
            void                                                resize(size_type newSize);
            void                                                resize(size_type newSize, const_reference value); // must be copyable
            void                                                shrink_to_fit();

            // -------------------------------- elements modifiers

            template <class Predicate>
            void                                                remove_if(Predicate pred);
            void                                                reverse();
            void                                                unique();
            template <class BinaryPredicate> 
            void                                                unique(BinaryPredicate pred);
            template <class Predicate> 
            void                                                retain(Predicate);
            
            void                                                append(const vector& vec);
            void                                                append(vector&& vec);
            void                                                append(const_iterator pos, const vector& vec);
            void                                                append(const_iterator pos, vector&& vec);

            // -------------------------------- elements

            template <typename ...Args>
            void                                                emplace_back(Args&&... args);
            template <typename TT>
            requires std::is_convertible_v<TT, T>
            void                                                push_back(TT&& value);
            void                                                pop_back() noexcept;

            template <typename TT>
            requires std::is_convertible_v<TT, T>
            void                                                insert(size_type index, TT&& value);
            template <typename ...Args>
            void                                                emplace(size_type index, Args&&... args);
            void                                                erase(size_type index);

            template <typename TT>
            requires std::is_convertible_v<TT, T>
            void insert(iterator it, TT&& value);
            template <typename ...Args>
            void                                                emplace(iterator it, Args&&... args);
            void                                                erase(iterator it);

            void                                                assign(size_type count, const_reference value);
    };
};

// #####################################################################################################################
// ----------------------------------------------- PRIVATE / HIDDEN METHODS ------------------------------------------//
// #####################################################################################################################

template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::__grow(size_type increase)
{
    if (m_size + increase <= m_capacity) return;

    size_type required = m_size + increase;
    size_type newCapacity = jman::max(m_capacity * GROW_FACTOR, required);
    newCapacity = jman::max(newCapacity, MIN_CAPACITY);

    if JMAN_UNLIKELY(!jman::is_safe_mul(sizeof(T), newCapacity)) 
        JMAN_THROW(std::overflow_error("jman::vector<T>::__grow() failed: calculated capacity overflows the size_type specified."));

    T* ptr = static_cast<T*>(::operator new(sizeof(T) * newCapacity, std::align_val_t{alignof(T)}));
    size_type constructed = 0;

    auto guard = scope_guard{[&](){
        if constexpr (!std::is_nothrow_move_constructible_v<T> && !std::is_nothrow_copy_constructible_v<T>) {
            jman::destruct_at(constructed, ptr);
            ::operator delete(static_cast<void*>(ptr), std::align_val_t{alignof(T)});
        }
    }};

    for (size_type i = 0; i < m_size; ++i) {
        jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
        jman::destruct_at(m_buffer + i);
        ++constructed;
    }
    
    if (m_buffer) ::operator delete(static_cast<void*>(m_buffer), std::align_val_t{alignof(T)});

    m_buffer = ptr;
    m_capacity = newCapacity;
    guard.dismiss();
}

// #####################################################################################################################
// -------------------------------------------- CONSTRUCTORS / DESTRUCTORS ------------------------------------------//
// #####################################################################################################################

template <typename T, std::unsigned_integral S>
jman::vector<T, S>::vector(std::initializer_list<T> list)
{   
    if (list.size() <= 0) return;

    size_type newCapacity = jman::max(static_cast<size_type>(list.size()), MIN_CAPACITY);
    m_buffer = static_cast<T*>(::operator new(sizeof(T) * newCapacity, std::align_val_t{alignof(T)}));

    auto guard = scope_guard{[&](){
        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            jman::destruct_at(m_size, m_buffer);
            ::operator delete(static_cast<void*>(m_buffer), std::align_val_t{alignof(T)});

            m_buffer = nullptr;
            m_size = 0;
        }
    }};

    for (const T& value: list) {
        jman::construct_at<T>(m_buffer + m_size, value);
        ++m_size;
    }

    m_capacity = newCapacity;
    guard.dismiss();
}

template <typename T, std::unsigned_integral S>
jman::vector<T, S>::vector(size_type size)
{
    if (size <= 0) return;

    size_type newCapacity = jman::max(static_cast<size_type>(size), MIN_CAPACITY);
    m_buffer = static_cast<T*>(::operator new(sizeof(T) * newCapacity, std::align_val_t{alignof(T)}));

    auto guard = scope_guard{[&](){
        if constexpr (!std::is_nothrow_default_constructible_v<T>) {
            jman::destruct_at(m_size, m_buffer);
            ::operator delete(static_cast<void*>(m_buffer), std::align_val_t{alignof(T)});

            m_buffer = nullptr;
            m_size = 0;
        }
    }};

    for (size_type i = 0; i < size; ++i) {
        jman::construct_at<T>(m_buffer + m_size);
        ++m_size;
    }

    m_capacity = newCapacity;
    guard.dismiss();
}

template <typename T, std::unsigned_integral S>
jman::vector<T, S>::vector(size_type size, const T& value)
{
    if (size <= 0) return;

    size_type newCapacity = jman::max(static_cast<size_type>(size), MIN_CAPACITY);
    m_buffer = static_cast<T*>(::operator new(sizeof(T) * newCapacity, std::align_val_t{alignof(T)}));

    auto guard = scope_guard{[&](){
        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            jman::destruct_at(m_size, m_buffer);
            ::operator delete(static_cast<void*>(m_buffer), std::align_val_t{alignof(T)});

            m_buffer = nullptr;
            m_size = 0;
        }
    }};

    for (size_type i = 0; i < size; ++i) {
        jman::construct_at<T>(m_buffer + m_size, value);
        ++m_size;
    }

    m_capacity = newCapacity;
    guard.dismiss();
}


template <typename T, std::unsigned_integral S>
jman::vector<T, S>::vector(const vector& other)
{
    if (other.m_size <= 0) return;

    size_type newCapacity = other.m_capacity;
    m_buffer = static_cast<T*>(::operator new(sizeof(T) * newCapacity, std::align_val_t{alignof(T)}));

    auto guard = scope_guard{[&](){
        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            jman::destruct_at(m_size, m_buffer);
            ::operator delete(static_cast<void*>(m_buffer), std::align_val_t{alignof(T)});

            m_buffer = nullptr;
            m_size = 0;
        }
    }};

    for (size_type i = 0; i < other.m_size; ++i) {
        jman::construct_at<T>(m_buffer + i, *std::launder(other.m_buffer + i));
        ++m_size;
    }

    m_capacity = newCapacity;
}

template <typename T, std::unsigned_integral S>
jman::vector<T, S>& jman::vector<T, S>::operator=(const vector& other)
{
    if (other.m_size <= 0) return *this;

    if (this != &other)
    {
        // if the buffer was allocated to fit the new vector's elements, just delete it and the old buffer remains intact
        // but if the buffer is reused, dont do anything and the buffer will remain in a mid/weird state

        if (m_capacity < other.m_capacity) {
            // allocate a new block
            T* buff = static_cast<T*>(::operator new(sizeof(T) * other.m_capacity, std::align_val_t{alignof(T)}));
            size_type size = 0;

            auto guard = scope_guard{[&](){
                if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
                    jman::destruct_at(size, buff);
                    ::operator delete(static_cast<void*>(buff), std::align_val_t{alignof(T)});
                }
            }};

            for (size_type i = 0; i < other.m_size; ++i) {
                jman::construct_at<T>(buff + i, *std::launder(other.m_buffer + i));
                ++size;
            }

            clear();
            if (m_buffer) ::operator delete(static_cast<void*>(m_buffer), std::align_val_t{alignof(T)});

            m_buffer = buff;
            m_capacity = other.m_capacity;
            m_size = size;
            guard.dismiss();
        } else {
            // reuse old memory block

            for (size_type i = 0; i < m_size; ++i)
                if (i < other.m_size)
                    *std::launder(m_buffer + i) = *std::launder(other.m_buffer + i);
                else
                    jman::destruct_at(std::launder(m_buffer + i));

            m_size = other.m_size;
        }
    }

    return *this;
}



template <typename T, std::unsigned_integral S>
jman::vector<T, S>::vector(vector&& other) noexcept
{
    m_buffer = other.m_buffer;
    m_capacity = other.m_capacity;
    m_size = other.m_size;

    other.m_buffer = nullptr;
    other.m_capacity = 0;
    other.m_size = 0;
}

template <typename T, std::unsigned_integral S>
jman::vector<T, S>& jman::vector<T, S>::operator=(vector&& other) noexcept
{
    if (this != &other)
    {
        clear();
        if (m_buffer) ::operator delete(static_cast<void*>(m_buffer), std::align_val_t{alignof(T)});

        m_buffer = other.m_buffer;
        m_capacity = other.m_capacity;
        m_size = other.m_size;

        other.m_buffer = nullptr;
        other.m_capacity = 0;
        other.m_size = 0;
    }

    return *this;
}





template <typename T, std::unsigned_integral S>
jman::vector<T, S>::~vector() noexcept
{
    clear();
    if (m_buffer) ::operator delete(static_cast<void*>(m_buffer), std::align_val_t{alignof(T)});
    m_buffer = nullptr;
    m_capacity = 0;
}

// ###################################################################################################
// -------------------------------------------- MODIFIERS ------------------------------------------//
// ###################################################################################################

template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::clear() noexcept
{
    jman::destruct_at(m_buffer, m_size);
    m_size = 0;
}

template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::reserve(size_type count)
{
    if (count <= m_capacity) return;

    T* reserved = static_cast<T*>(::operator new(sizeof(T) * count, std::align_val_t{alignof(T)}));
    size_type constructed = 0;
    
    auto guard = scope_guard{[&](){
        if constexpr (!std::is_nothrow_move_constructible_v<T> && !std::is_nothrow_copy_constructible_v<T>) {
            jman::destruct_at(constructed, reserved);
            ::operator delete(static_cast<void*>(reserved), std::align_val_t{alignof(T)});
        }
    }};

    for (size_type i = 0; i < m_size; ++i) {
        jman::construct_at<T>(reserved + i, std::move_if_noexcept(m_buffer[i]));
        jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
        ++constructed;
    }

    if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

    m_buffer = reserved;
    m_capacity = count;
    guard.dismiss();
}

template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::resize(size_type newSize)
{
    if (newSize > m_size) 
    // increase the capacity of the vector and replace the empty gaps with default constructed elements
    {
        T* ptr = reinterpret_cast<T*>(::operator new(sizeof(T) * newSize, std::align_val_t{alignof(T)}));
        size_type constructed = 0;
    
        auto guard = scope_guard{[&](){
            if constexpr (
                !std::is_nothrow_move_constructible_v<T> && 
                !std::is_nothrow_copy_constructible_v<T> &&
                !std::is_nothrow_default_constructible_v<T>
            ) {
                jman::destruct_at(constructed, ptr);
                ::operator delete(static_cast<void*>(ptr), std::align_val_t{alignof(T)});
            }
        }};

        for (size_type i = 0; i < m_size; ++i) {
            jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
            jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
            ++constructed;
        }
        for (size_type i = m_size; i < newSize; ++i) {
            jman::construct_at<T>(ptr + i);
            ++constructed;
        }

        if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

        m_buffer = ptr;
        m_size = m_capacity = newSize;
        guard.dismiss();
    } 
    else if (newSize < m_size) 
    // decrease the contents and capacity of the vector
    {
        for (size_type i = m_size - 1; i >= newSize; --i)
            jman::destruct_at(m_buffer + i);
    
        if (newSize == 0) {
            ::operator delete(m_buffer);
            m_buffer = nullptr;
            m_size = m_capacity = 0;
            return;
        }

        T* ptr = reinterpret_cast<T*>(::operator new(sizeof(T) * newSize, std::align_val_t{alignof(T)}));
        size_type constructed = 0;

        auto guard = scope_guard{[&](){
            if constexpr (
                !std::is_nothrow_move_constructible_v<T> && 
                !std::is_nothrow_copy_constructible_v<T>
            ) {
                jman::destruct_at(constructed, ptr);
                ::operator delete(static_cast<void*>(ptr), std::align_val_t{alignof(T)});
            }
        }};

        for (size_type i = 0; i < newSize; ++i) {
            jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
            jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
            ++constructed;
        }

        if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

        m_buffer = ptr;
        m_size = m_capacity = newSize;
    }
}

template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::resize(S newSize, const T& value)
{
    if (newSize > m_size) 
    // increase the capacity of the vector and replace the empty gaps with default constructed elements
    {
        T* ptr = reinterpret_cast<T*>(::operator new(sizeof(T) * newSize, std::align_val_t{alignof(T)}));
        size_type constructed = 0;
    
        auto guard = scope_guard{[&](){
            if constexpr (
                !std::is_nothrow_move_constructible_v<T> && 
                !std::is_nothrow_copy_constructible_v<T>
            ) {
                jman::destruct_at(constructed, ptr);
                ::operator delete(static_cast<void*>(ptr), std::align_val_t{alignof(T)});
            }
        }};

        for (size_type i = 0; i < m_size; ++i) {
            jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
            jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
            ++constructed;
        }
        for (size_type i = m_size; i < newSize; ++i) {
            jman::construct_at<T>(ptr + i, value);
            ++constructed;
        }

        if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

        m_buffer = ptr;
        m_size = m_capacity = newSize;
        guard.dismiss();
    } 
    else if (newSize < m_size) 
    // decrease the contents and capacity of the vector
    {
        for (size_type i = m_size - 1; i >= newSize; --i)
            jman::destruct_at(m_buffer + i);
    
        if (newSize == 0) {
            ::operator delete(m_buffer);
            m_buffer = nullptr;
            m_size = m_capacity = 0;
            return;
        }

        T* ptr = reinterpret_cast<T*>(::operator new(sizeof(T) * newSize, std::align_val_t{alignof(T)}));
        size_type constructed = 0;

        auto guard = scope_guard{[&](){
            if constexpr (
                !std::is_nothrow_move_constructible_v<T> && 
                !std::is_nothrow_copy_constructible_v<T>
            ) {
                jman::destruct_at(constructed, ptr);
                ::operator delete(static_cast<void*>(ptr), std::align_val_t{alignof(T)});
            }
        }};

        for (size_type i = 0; i < newSize; ++i) {
            jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
            jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
            ++constructed;
        }

        if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

        m_buffer = ptr;
        m_size = m_capacity = newSize;
    }
}

template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::shrink_to_fit()
{
    if (m_size == m_capacity) return;
    resize(m_size);
}

// ###################################################################################################
// -------------------------------------------- ELEMENTS -------------------------------------------//
// ###################################################################################################

template <typename T, std::unsigned_integral S>
template <typename ...Args>
void jman::vector<T, S>::emplace_back(Args&&... args)
{
    __grow();
    jman::construct_at<T>(m_buffer + m_size, std::forward<Args>(args)...);
    ++m_size;
}
template <typename T, std::unsigned_integral S>
template <typename TT>
requires std::is_convertible_v<TT, T>
void jman::vector<T, S>::push_back(TT&& value)
{
    __grow();
    jman::construct_at<T>(m_buffer + m_size, static_cast<T>(std::forward<TT>(value)));
    ++m_size;
}

template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::pop_back() noexcept
{
    if (empty()) return;
    jman::destruct_at(m_buffer + m_size);
    --m_size;
}

template <typename T, std::unsigned_integral S>
template <typename TT>
requires std::is_convertible_v<TT, T>
void jman::vector<T, S>::insert(size_type index, TT&& value)
{
    if (index >= m_size) throw std::out_of_range("jman::vector<T>::insert() failed: Index out of range.");
    __grow();

    size_type moved = 0;

    auto guard = scope_guard{[&](){
        if constexpr (
            !std::is_nothrow_move_assignable_v<T> && 
            !std::is_nothrow_copy_assignable_v<T> &&
            !std::is_nothrow_default_constructible_v<T>
        )
            for (size_type i = moved; i < m_size; --i)
                m_buffer[i] = std::move_if_noexcept(m_buffer[i + 1]);
    }};

    for (size_type i = m_size; i > index; --i) {
        m_buffer[i] = std::move_if_noexcept(m_buffer[i - 1]);
        ++moved;
    }

    
    jman::destruct_at(m_buffer + index);
    jman::construct_at<T>(m_buffer + index, static_cast<T>(std::forward<TT>(value)));
    guard.dismiss();
    ++m_size;
}
template <typename T, std::unsigned_integral S>
template <typename ...Args>
void jman::vector<T, S>::emplace(size_type index, Args&&... args)
{
    if (index >= m_size || index < 0) throw std::out_of_range("jman::vector<T>::emplace() failed: Index out of range.");
    __grow();

    size_type moved = 0;

    auto guard = scope_guard{[&](){
        if constexpr (
            !std::is_nothrow_move_assignable_v<T> && 
            !std::is_nothrow_copy_assignable_v<T> && 
            !std::is_nothrow_constructible_v<T, Args...>
        )
            for (size_type i = moved; i < m_size; --i)
                m_buffer[i] = std::move_if_noexcept(m_buffer[i + 1]);
    }};

    for (size_type i = m_size; i > index; --i) {
        m_buffer[i] = std::move_if_noexcept(m_buffer[i - 1]);
        ++moved;
    }

    jman::destruct_at(m_buffer + index);
    jman::construct_at<T>(m_buffer + index, std::forward<Args>(args)...);
    guard.dismiss();
    ++m_size;
}
template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::erase(size_type index)
{
    if (index >= m_size || index < 0) throw std::out_of_range("jman::vector<T>::erase() failed: Index out of range.");
    size_type moved = 0;

    auto guard = scope_guard{[&](){
        if constexpr (!std::is_nothrow_move_assignable_v<T> && !std::is_nothrow_copy_assignable_v<T>)
            for (size_type i = m_size; i > moved; --i)
                m_buffer[i] = std::move_if_noexcept(m_buffer[i - 1]);
    }};

    for (size_type i = index; i < m_size; ++i) {
        m_buffer[i] = std::move_if_noexcept(m_buffer[i + 1]);
        ++moved;
    }
    
    guard.dismiss();
    jman::destruct_at(m_buffer + m_size);
    --m_size;
}


template <typename T, std::unsigned_integral S>
template <typename TT>
requires std::is_convertible_v<TT, T>
void jman::vector<T, S>::insert(iterator it, TT&& value)
{
    size_type index = it - begin();
    insert(index, std::forward<TT>(value));
}
template <typename T, std::unsigned_integral S>
template <typename ...Args>
void jman::vector<T, S>::emplace(iterator it, Args&&... args)
{
    size_type index = it - begin();
    emplace(index, std::forward<Args>(args)...);
}
template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::erase(iterator it)
{
    size_type index = it - begin();
    erase(index);
}

template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::assign(size_type count, const T& value)
{
    /* 
    it could be done by constructing and using the copy assignment operator on the elements in one loop
     but i think its easier to just clear the elements first to avoid moves or copies during growing
    */
    clear(); 
    if (count > m_capacity) __grow(count - m_capacity);

    for (size_type i = 0; i < count; ++i) {
        jman::construct_at<T>(m_buffer + i, value);
        ++m_size;
    }
}