#pragma once

#include <cstddef>
#include <limits>
#include <new>
#include <type_traits>
#include <optional>
#include <stdexcept>
#include <justaman/memory/lifetime.hpp>
#include <justaman/utils/attributes.h>
#include <justaman/algorithms/numeric.hpp>

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
        Exception handling:
            This class checks in compile time if moving, copying constructing an element is marked as noexcept, if so, operations will run normally.
            If not, operations will run in a try catch block, when an exception is catched, it will try to undo the operations and rethrow the exception to the caller
            
    */
    template <typename T, std::unsigned_integral S = std::size_t>
    class vector 
    {
        public: 
            // for std:: methods to work (it also needs allocator_type but since i use defaut malloc and most dont neeed it so yea)
            using value_type = T;
            using size_type = S;
            using difference_type = std::ptrdiff_t;
            using reference = T&;
            using const_reference = const T&;
            using pointer = T*;
            using const_pointer = const T*;

            constexpr static size_type DEFAULT_CAPACITY = 0;
            constexpr static size_type GROW_FACTOR = 2;

        private:
            T* m_buffer = nullptr;

            size_type m_capacity = DEFAULT_CAPACITY;
            size_type m_size = 0;

        private:
            void grow(size_type increase = 1) 
            {
                if (m_size + increase <= m_capacity) return;

                size_type required = m_size + increase;
                size_type newCapacity = jman::max(m_capacity * GROW_FACTOR, required);
                if JMAN_UNLIKELY(newCapacity < 16) newCapacity = 16;

                if JMAN_UNLIKELY(!jman::is_safe_mul(sizeof(T), newCapacity)) 
                    throw std::overflow_error("jman::vector<T>::grow() failed: calculated capacity overflows the size_type specified.");

                T* ptr = reinterpret_cast<T*>(::operator new(sizeof(T) * newCapacity, std::align_val_t{alignof(T)}));
                
                if constexpr (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_copy_constructible_v<T>) {
                    // 100% safe to move or copy without risks for exceptions

                    for (size_type i = 0; i < m_size; ++i) {
                        jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
                        jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
                    }
                    
                    if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});
                    
                    m_buffer = ptr;
                    m_capacity = newCapacity;
                } else {
                    // not safe to move or copy
                    size_type constructed = 0;

                    try {
                        for (size_type i = 0; i < m_size; ++i) {
                            jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
                            jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
                            ++constructed;
                        }
                        
                        if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

                        m_buffer = ptr;
                        m_capacity = newCapacity;
                    } catch (...) {
                        jman::destruct_at(constructed, ptr);
                        ::operator delete(ptr, std::align_val_t{alignof(T)});

                        throw;
                    }
                }
            }

        public:
            struct const_iterator 
            {
                private: 
                    const T* m_ptr = nullptr;

                public:
                    using iterator_concept  = std::contiguous_iterator_tag;
                    using iterator_category = std::random_access_iterator_tag;
                    using value_type        = T;
                    using difference_type   = std::ptrdiff_t;
                    using reference = const T&;
                    using pointer = const T*;

                    const_iterator() = default;
                    explicit const_iterator(T* ptr) : m_ptr(ptr) {}

                    JMAN_FORCE_INLINE reference operator[](difference_type index) const { return m_ptr[index]; }

                    JMAN_FORCE_INLINE const_iterator& operator++() const noexcept { m_ptr += 1; return *this; }
                    JMAN_FORCE_INLINE const_iterator operator++(int) const noexcept { iterator retval = *this; ++(*this); return retval; }
                    JMAN_FORCE_INLINE const_iterator& operator--() const noexcept { m_ptr -= 1; return *this; }
                    JMAN_FORCE_INLINE const_iterator operator--(int) const noexcept { iterator retval = *this; --(*this); return retval; }
                    
                    JMAN_FORCE_INLINE const_iterator operator+(difference_type index) const noexcept { return const_iterator(m_ptr + index); }
                    JMAN_FORCE_INLINE const_iterator operator-(difference_type index) const noexcept { return const_iterator(m_ptr - index); }
                    
                    JMAN_FORCE_INLINE difference_type operator-(const const_iterator& other) const noexcept { return m_ptr - other.m_ptr; }
                    
                    JMAN_FORCE_INLINE const_iterator& operator+=(difference_type diff) const noexcept { m_ptr += diff; return *this; }
                    JMAN_FORCE_INLINE const_iterator& operator-=(difference_type diff) const noexcept { m_ptr -= diff; return *this; }
                    friend JMAN_FORCE_INLINE const_iterator operator+(difference_type diff, const_iterator it) noexcept { return {diff + it.m_ptr}; }

                    JMAN_FORCE_INLINE pointer operator->() const { return std::launder(m_ptr); }
                    JMAN_FORCE_INLINE reference operator*() const { return *std::launder(m_ptr); }

                    auto operator <=>(const const_iterator&) const = default;
            };

            struct iterator 
            {
                private:
                    T* m_ptr = nullptr;

                public:
                    using iterator_concept  = std::contiguous_iterator_tag;
                    using iterator_category = std::random_access_iterator_tag;
                    using value_type        = T;
                    using difference_type   = std::ptrdiff_t;
                    using pointer           = T*;
                    using reference         = T&;

                    iterator() = default;
                    explicit iterator(T* ptr) : m_ptr(ptr) {}
                    
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
            

            constexpr vector();
            explicit vector(std::initializer_list<T> list);

            vector(const vector& other);
            vector& operator=(const vector& other);

            vector(vector&& other) noexcept;
            vector& operator=(vector&& other) noexcept;

            ~vector() noexcept;

            // ----------------- GETTERS ------------- //

            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type       capacity() const noexcept { return m_capacity; }
            /** @warning returned value is an aproximation. */
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type       max_size() const noexcept { return std::numeric_limits<size_type>::max() / sizeof(T); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type       size() const noexcept { return m_size; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool            empty() const noexcept { return m_size == 0; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE pointer         data() noexcept { return std::launder(m_buffer); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference       front() noexcept { return *std::launder(m_buffer); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference       back() noexcept { return *std::launder(m_buffer + m_size - 1); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference       at(size_type index) noexcept {
                if (index >= m_size || index < 0) throw std::out_of_range("jman::vector<T>::operator[]() failed: Out of bounds when trying to index an object.");
                return *std::launder(m_buffer + index);
            }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference        operator[](size_type index) noexcept { return *std::launder(m_buffer + index);; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_pointer    data() const noexcept { return std::launder(m_buffer); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference  front() const noexcept { return *std::launder(m_buffer); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference  back() const noexcept { return *std::launder(m_buffer + m_size - 1); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference  at(size_type index) const noexcept {
                if (index >= m_size || index < 0) throw std::out_of_range("jman::vector<T>::operator[]() failed: Out of bounds when trying to index an object.");
                return *std::launder(m_buffer + index);
            }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference  operator[](size_type index) const noexcept { return *std::launder(m_buffer + index);; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator begin() noexcept { return iterator{m_buffer}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator end() noexcept { return iterator{m_buffer + m_size}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator begin() const noexcept { return const_iterator{m_buffer}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator end() const noexcept { return const_iterator{m_buffer + m_size}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator cbegin() const noexcept { return const_iterator {m_buffer}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator cend() const noexcept { return const_iterator {m_buffer + m_size}; }

            // ----------------- MODIFIERS ------------- //

            void clear() noexcept;
            void reserve(size_type count);
            void resize(size_type newSize);
            void resize(size_type newSize, const_reference value); // must be copyable
            void shrink_to_fit();

            template <typename ...Args>
            void emplace_back(Args&&... args);
            template <typename TT>
            requires std::is_convertible_v<TT, T>
            void push_back(TT&& value);
            void pop_back() noexcept;
            void swap_and_pop(size_type index);
            void swap_and_pop(iterator it);
            std::optional<T> pop_back_safe();

            template <typename TT>
            requires std::is_convertible_v<TT, T>
            void insert(size_type index, TT&& value);
            template <typename ...Args>
            void emplace(size_type index, Args&&... args);
            void erase(size_type index);

            template <typename TT>
            requires std::is_convertible_v<TT, T>
            void insert(iterator it, TT&& value);
            template <typename ...Args>
            void emplace(iterator it, Args&&... args);
            void erase(iterator it);

            void assign(size_type count, const T& value);
    };
};

// #####################################################################################################################
// -------------------------------------------- CONSTRUCTORS / DESTRUCTORS ------------------------------------------//
// #####################################################################################################################


template <typename T, std::unsigned_integral S>
constexpr jman::vector<T, S>::vector()
{
    if (DEFAULT_CAPACITY > 0)
        m_buffer = reinterpret_cast<T*>(::operator new(sizeof(T) * DEFAULT_CAPACITY, std::align_val_t{alignof(T)}));
}

template <typename T, std::unsigned_integral S>
jman::vector<T, S>::vector(std::initializer_list<T> list)
{
    m_capacity = static_cast<size_type>(list.size());
    m_buffer = reinterpret_cast<T*>(::operator new(sizeof(T) * m_capacity, std::align_val_t{alignof(T)}));

    if constexpr (std::is_nothrow_copy_constructible_v<T>) {
        for (const T& value: list) {
            jman::construct_at<T>(m_buffer + m_size, value);
            ++m_size;
        }
    } else {
        try {
            for (const T& value: list) {
                jman::construct_at<T>(m_buffer + m_size, value);
                ++m_size;
            }
        } catch (...) {
            jman::destruct_at(m_size, m_buffer);
            ::operator delete(m_buffer, std::align_val_t{alignof(T)});

            m_buffer = nullptr;
            m_capacity = m_size = 0;

            throw;
        }
    }
}


template <typename T, std::unsigned_integral S>
jman::vector<T, S>::vector(const vector& other)
{
    m_capacity = other.m_capacity;
    m_buffer = reinterpret_cast<T*>(::operator new(sizeof(T) * m_capacity, std::align_val_t{alignof(T)}));

    if constexpr (std::is_nothrow_copy_constructible_v<T>) {
        for (size_type i = 0; i < other.m_size; ++i) {
            jman::construct_at<T>(m_buffer + i, other.m_buffer[i]);
            ++m_size;
        }
    } else {
        try {
            for (size_type i = 0; i < other.m_size; ++i) {
                jman::construct_at<T>(m_buffer + i, other.m_buffer[i]);
                ++m_size;
            }
        } catch (...) {
            jman::destruct_at(m_size, m_buffer);
            ::operator delete(m_buffer, std::align_val_t{alignof(T)});

            m_buffer = nullptr;
            m_capacity = 0;

            throw;
        }
    }
}

template <typename T, std::unsigned_integral S>
jman::vector<T, S>& jman::vector<T, S>::operator=(const vector& other)
{
    if (this != &other)
    {
        T* buff = reinterpret_cast<T*>(::operator new(sizeof(T) * other.m_capacity, std::align_val_t{alignof(T)}));

        if constexpr (std::is_nothrow_copy_constructible_v<T>) {
            for (size_type i = 0; i < other.m_size; ++i) {
                jman::construct_at<T>(buff + i, other.m_buffer[i]);
                ++m_size;
            }

            clear();
            if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

            m_buffer = buff;
            m_capacity = other.m_capacity;
        } else {
            size_type size = 0;
            try {
                for (size_type i = 0; i < other.m_size; ++i) {
                    jman::construct_at<T>(buff + i, other.m_buffer[i]);
                    ++size;
                }

                clear();
                if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

                m_buffer = buff;
                m_size = size;
                m_capacity = other.m_capacity;
            } catch (...) {
                jman::destruct_at(size, buff);
                ::operator delete(buff, std::align_val_t{alignof(T)});

                throw;
            }
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
        if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

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
    if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});
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

    T* reserved = reinterpret_cast<T*>(::operator new(sizeof(T) * count, std::align_val_t{alignof(T)}));
    
    if constexpr (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_copy_constructible_v<T>) {
        for (size_type i = 0; i < m_size; ++i) {
            jman::construct_at<T>(reserved + i, std::move_if_noexcept(m_buffer[i]));
            jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
        }
        
        if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

        m_buffer = reserved;
        m_capacity = count;
    } else {
        size_type constructed = 0;

        try {
            for (size_type i = 0; i < m_size; ++i) {
                jman::construct_at<T>(reserved + i, std::move_if_noexcept(m_buffer[i]));
                jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
                ++constructed;
            }
            
            if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

            m_buffer = reserved;
            m_capacity = count;
        } catch (...) {
            jman::destruct_at(constructed, reserved);
            ::operator delete(reserved, std::align_val_t{alignof(T)});

            throw;
        }
    }
}

template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::resize(size_type newSize)
{
    if (newSize > m_size) // increase
    {
        T* ptr = reinterpret_cast<T*>(::operator new(sizeof(T) * newSize, std::align_val_t{alignof(T)}));

        if constexpr (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_copy_constructible_v<T>) {
            for (size_type i = 0; i < m_size; ++i) {
                jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
                jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
            }
            for (size_type i = m_size; i < newSize; ++i)
                jman::construct_at<T>(ptr + i);


            if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

            m_buffer = ptr;
            m_size = m_capacity = newSize;
        } else {
            size_type constructed = 0;

            try {
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
            } catch (...) {
                jman::destruct_at(constructed, ptr);
                ::operator delete(ptr, std::align_val_t{alignof(T)});

                throw;
            }
        }
    } 
    else if (newSize < m_size) // decrease
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

        if constexpr (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_copy_constructible_v<T>) {
            for (size_type i = 0; i < newSize; ++i) {
                jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
                jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
            }

            if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

            m_buffer = ptr;
            m_size = m_capacity = newSize;
        } else {
            size_type constructed = 0;
        
            try {
                for (size_type i = 0; i < newSize; ++i) {
                    jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
                    jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
                    ++constructed;
                }

                if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

                m_buffer = ptr;
                m_size = m_capacity = newSize;
            } catch(...) {
                jman::destruct_at(constructed, ptr);
                ::operator delete(ptr, std::align_val_t{alignof(T)});

                throw;
            }
        }
    }
}

template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::resize(S newSize, const T& value)
{
    if (newSize > m_size) // increase
    {
        T* ptr = reinterpret_cast<T*>(::operator new(sizeof(T) * newSize, std::align_val_t{alignof(T)}));

        if constexpr (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_copy_constructible_v<T>) {
            for (size_type i = 0; i < m_size; ++i) {
                jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
                jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
            }
            for (size_type i = m_size; i < newSize; ++i)
                jman::construct_at<T>(ptr + i);


            if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

            m_buffer = ptr;
            m_size = m_capacity = newSize;
        } else {
            size_type constructed = 0;

            try {
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
            } catch (...) {
                jman::destruct_at(constructed, ptr);
                ::operator delete(ptr, std::align_val_t{alignof(T)});

                throw;
            }
        }
    } 
    else if (newSize < m_size) // decrease
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

        if constexpr (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_copy_constructible_v<T>) {
            for (size_type i = 0; i < newSize; ++i) {
                jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
                jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
            }

            if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

            m_buffer = ptr;
            m_size = m_capacity = newSize;
        } else {
            size_type constructed = 0;
        
            try {
                for (size_type i = 0; i < newSize; ++i) {
                    jman::construct_at<T>(ptr + i, std::move_if_noexcept(m_buffer[i]));
                    jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
                    ++constructed;
                }

                if (m_buffer) ::operator delete(m_buffer, std::align_val_t{alignof(T)});

                m_buffer = ptr;
                m_size = m_capacity = newSize;
            } catch(...) {
                jman::destruct_at(constructed, ptr);
                ::operator delete(ptr, std::align_val_t{alignof(T)});

                throw;
            }
        }
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
    grow();
    jman::construct_at<T>(m_buffer + m_size, std::forward<Args>(args)...);
    ++m_size;
}
template <typename T, std::unsigned_integral S>
template <typename TT>
requires std::is_convertible_v<TT, T>
void jman::vector<T, S>::push_back(TT&& value)
{
    grow();
    jman::construct_at<T>(m_buffer + m_size, static_cast<T>(std::forward<TT>(value)));
    ++m_size;
}

template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::pop_back() noexcept
{
    if (empty()) return;
    --m_size;
    jman::destruct_at(m_buffer + m_size);
}

template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::swap_and_pop(size_type index)
{
    if (empty() || index >= m_size) throw std::runtime_error("jman::vector<T>::swap_and_pop() failed: Attempted to swap_and_pop an empty vector or index out of range");
    m_buffer[index] = std::move_if_noexcept(back()); // no need for try and catch blocks, if it thorws mid moving or copying, too bad
    pop_back();
}

template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::swap_and_pop(iterator it)
{
    size_type index = it - begin();
    swap_and_pop(index);
}

template <typename T, std::unsigned_integral S>
std::optional<T> jman::vector<T, S>::pop_back_safe()
{
    if (empty()) { return std::nullopt; }
    T last_element = std::move_if_noexcept(back());
    pop_back();
    return last_element;
}

template <typename T, std::unsigned_integral S>
template <typename TT>
requires std::is_convertible_v<TT, T>
void jman::vector<T, S>::insert(size_type index, TT&& value)
{
    if (index >= m_size) throw std::out_of_range("jman::vector<T>::insert() failed: Index out of range.");
    grow();

    if constexpr (std::is_nothrow_move_assignable_v<T> || std::is_nothrow_copy_assignable_v<T>) {
        for (size_type i = m_size; i > index; --i)
            m_buffer[i] = std::move_if_noexcept(m_buffer[i - 1]);
    } else {
        size_type moved = 0;

        try {
            for (size_type i = m_size; i > index; --i) {
                m_buffer[i] = std::move_if_noexcept(m_buffer[i - 1]);
                ++moved;
            }
        } catch(...) {
            for (size_type i = moved; i < m_size; --i)
                m_buffer[i] = std::move_if_noexcept(m_buffer[i + 1]);
            
            throw;
        };
    }
            
    jman::destruct_at(m_buffer + index);
    jman::construct_at<T>(m_buffer + index, static_cast<T>(std::forward<TT>(value)));
    ++m_size;
}
template <typename T, std::unsigned_integral S>
template <typename ...Args>
void jman::vector<T, S>::emplace(size_type index, Args&&... args)
{
    if (index >= m_size || index < 0) throw std::out_of_range("jman::vector<T>::emplace() failed: Index out of range.");
    grow();

    if constexpr (std::is_nothrow_move_assignable_v<T> || std::is_nothrow_copy_assignable_v<T>) {
        for (size_type i = m_size; i > index; --i)
            m_buffer[i] = std::move_if_noexcept(m_buffer[i - 1]);
    } else {
        size_type moved = 0;

        try {
            for (size_type i = m_size; i > index; --i) {
                m_buffer[i] = std::move_if_noexcept(m_buffer[i - 1]);
                ++moved;
            }
        } catch(...) {
            for (size_type i = moved; i < m_size; --i)
                m_buffer[i] = std::move_if_noexcept(m_buffer[i + 1]);

            throw;
        };
    }

    jman::destruct_at(m_buffer + index);
    jman::construct_at<T>(m_buffer + index, std::forward<Args>(args)...);
    ++m_size;
}
template <typename T, std::unsigned_integral S>
void jman::vector<T, S>::erase(size_type index)
{
    if (index >= m_size || index < 0) throw std::out_of_range("jman::vector<T>::erase() failed: Index out of range.");

    if constexpr (std::is_nothrow_move_assignable_v<T> || std::is_nothrow_copy_assignable_v<T>) {
        for (size_type i = index; i < m_size; ++i)
            m_buffer[i] = std::move_if_noexcept(m_buffer[i + 1]);
    } else {
        size_type moved = 0;

        try {
            for (size_type i = index; i < m_size; ++i) {
                m_buffer[i] = std::move_if_noexcept(m_buffer[i + 1]);
                ++moved;
            }
        } catch(...) {
            for (size_type i = m_size; i > moved; --i)
                m_buffer[i] = std::move_if_noexcept(m_buffer[i - 1]);

            throw;
        };
    }
    
    --m_size;
    jman::destruct_at(m_buffer + m_size);
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
    clear();
    if (count > m_capacity) grow(count - m_capacity);
    m_size = m_capacity;

    if constexpr (std::is_nothrow_copy_constructible_v<T>) {
        for (size_type i = 0; i < count; ++i) {
                jman::construct_at<T>(m_buffer + i, value);
                jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
            }
    } else {
        size_type constructed = 0;

        try {
            for (size_type i = 0; i < count; ++i) {
                jman::construct_at<T>(m_buffer + i, value);
                jman::destruct_at(m_buffer + i); // call destructor since the object might be copied
                ++constructed;
            }
        } catch (...) {
            jman::destruct_at(constructed, m_buffer);
            throw;
        }
    }
}