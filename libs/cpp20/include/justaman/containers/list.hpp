#pragma once

#include "justaman/utils/attributes.h"
#include <cstddef>
#include <initializer_list>
#include <limits>

namespace jman 
{
    template <typename T>
    class linked_list 
    {
        public:
            using value_type = T;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            using reference = T&;
            using const_reference = const T&;
            using pointer = T*;
            using const_pointer = const T*;

        private:
            struct node 
            {
                T value;
                
                node* next;
                node* prev;
            };

            node* m_head = nullptr;
            node* m_prev = nullptr;
            std::size_t m_size = 0;

        public:
            struct iterator
            {

            };

            struct const_iterator
            {

            };

            linked_list();
            explicit linked_list(std::size_t size);
            linked_list(std::size_t size, const T& value);
            linked_list(std::initializer_list<T> list);
            
            linked_list(const linked_list& other);
            linked_list& operator=(const linked_list& other);
            
            linked_list(linked_list&& other) noexcept;
            linked_list& operator=(linked_list&& other) noexcept;

            ~linked_list() noexcept;

            reference front();
            const_reference front() const;
            reference back();
            const_reference back() const;
            reference at();
            const_reference at() const;

            iterator begin();
            iterator end();
            const_iterator begin() const;
            const_iterator end() const;
            const_iterator cbegin() const;
            const_iterator cend() const;

            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool empty() { return m_size == 0; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type size() { return m_size == 0; }
            /** @warning: this method is just an aproximation */
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type      max_size() const noexcept { return std::numeric_limits<size_type>::max() / sizeof(node); }

            void clear();
            void resize();

            void push_front();
            void push_back();
            void emplace_front();
            void emplace_back();

            void pop_front();
            void pop_back();

            iterator insert(); 
            iterator emplace(); 
            void erase(); 
    };
}