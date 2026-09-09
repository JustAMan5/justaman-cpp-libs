#pragma once

#include "justaman/errors/safe_exceptions.hpp"
#include <justaman/memory/lifetime.hpp>
#include <justaman/utils/attributes.h>
#include <justaman/algorithms/numeric.hpp>
#include <justaman/utils/scope.hpp>
#include <iterator>
#include <limits>
#include <new>
#include <type_traits>
#include <stdexcept>

namespace jman 
{
    /**
        @brief
             my custom implementation of a fast and exception safe doubly circular linked list
        \n
        Iterators:
            iterators work the same as std::vector and have the same rules
    */
    template <typename T>
    class linked_list 
    {
        static_assert(!std::is_reference_v<T>, "Type T can't be a reference of any kind.");
        static_assert(!std::is_void_v<T>, "Type T can't be a of void type.");
        static_assert(std::is_nothrow_destructible_v<T>, "Type T must has it's destructor marked as noexcept for safety.");

        public:
            using value_type        = T;
            using size_type         = std::size_t;
            using difference_type   = std::ptrdiff_t;
            using reference         = T&;
            using const_reference   = const T&;
            using pointer           = T*;
            using const_pointer     = const T*;

        private:
            struct __list_node_base 
            {
                __list_node_base* next;
                __list_node_base* prev;
            };

            struct __list_node: public __list_node_base
            {
                alignas(T) std::byte buffer[sizeof(T)];
            };

            __list_node_base    m_dummy {};
            std::size_t         m_size = 0;

        private:
            JMAN_NO_DISCARD static JMAN_FORCE_INLINE T* __ptr(__list_node* node) noexcept { return std::launder(reinterpret_cast<T*>(&node->buffer)); }
            JMAN_NO_DISCARD static JMAN_FORCE_INLINE const T* __ptr(const __list_node* node) noexcept { return std::launder(reinterpret_cast<const T*>(&node->buffer)); }

            JMAN_NO_DISCARD static JMAN_FORCE_INLINE T* __ptr(__list_node_base* node) noexcept { return std::launder(reinterpret_cast<T*>(&static_cast<__list_node*>(node)->buffer)); }
            JMAN_NO_DISCARD static JMAN_FORCE_INLINE const T* __ptr(const __list_node_base* node) noexcept { return std::launder(reinterpret_cast<const T*>(&static_cast<const __list_node*>(node)->buffer)); }

            JMAN_NO_DISCARD static JMAN_FORCE_INLINE T& __value(__list_node* node) noexcept { return *std::launder(reinterpret_cast<T*>(&node->buffer)); }
            JMAN_NO_DISCARD static JMAN_FORCE_INLINE const T& __value(const __list_node* node) noexcept { return *std::launder(reinterpret_cast<const T*>(&node->buffer)); }

            JMAN_NO_DISCARD static JMAN_FORCE_INLINE T& __value(__list_node_base* node) noexcept { return *std::launder(reinterpret_cast<T*>(&static_cast<__list_node*>(node)->buffer)); }
            JMAN_NO_DISCARD static JMAN_FORCE_INLINE const T& __value(const __list_node_base* node) noexcept { return *std::launder(reinterpret_cast<const T*>(&static_cast<const __list_node*>(node)->buffer)); }

            void __clean_nodes_up_to_backwards(__list_node_base* node) noexcept;
            void __clean_nodes_up_to_forward(__list_node_base* node) noexcept;
            void __swap_nodes(__list_node_base* a, __list_node_base* b) noexcept;
            __list_node_base* __split() noexcept;
            template <typename Comp>
            __list_node_base* __sort_merge(__list_node_base* first, __list_node_base* second, Comp comp);

        public:
            struct const_iterator
            {
                private: 
                    friend class linked_list<T>;
                    const __list_node_base* m_ptr;
                
                public:
                    using iterator_category = std::bidirectional_iterator_tag;
                    using value_type        = T;
                    using difference_type   = std::ptrdiff_t;
                    using reference         = const T&;
                    using pointer           = const T*;

                    explicit const_iterator(__list_node_base* ptr) noexcept
                        : m_ptr(ptr) {}

                    JMAN_FORCE_INLINE const_iterator& operator++() noexcept { m_ptr = m_ptr->next; return *this; }
                    JMAN_FORCE_INLINE const_iterator operator++(int) noexcept { iterator retval = *this; ++(*this); return retval; }
                    JMAN_FORCE_INLINE const_iterator& operator--() noexcept { m_ptr = m_ptr->prev; return *this; }
                    JMAN_FORCE_INLINE const_iterator operator--(int) noexcept { iterator retval = *this; --(*this); return retval; }

                    JMAN_FORCE_INLINE pointer operator->() const { return __ptr(m_ptr); }
                    JMAN_FORCE_INLINE reference operator*() const { return __value(m_ptr); }

                    JMAN_FORCE_INLINE bool operator==(const const_iterator& other) const noexcept { return m_ptr == other.m_ptr; }
                    JMAN_FORCE_INLINE bool operator!=(const const_iterator& other) const noexcept { return m_ptr != other.m_ptr; }
            };

            struct iterator
            {
                private: 
                    friend class linked_list<T>;
                    __list_node_base* m_ptr;
                
                public:
                    using iterator_category = std::bidirectional_iterator_tag;
                    using value_type        = T;
                    using difference_type   = std::ptrdiff_t;
                    using reference         = T&;
                    using pointer           = T*;

                    explicit iterator(__list_node_base* ptr) noexcept
                        : m_ptr(ptr) {}
                    
                    JMAN_FORCE_INLINE iterator& operator++() noexcept { m_ptr = m_ptr->next; return *this; }
                    JMAN_FORCE_INLINE iterator operator++(int) noexcept { iterator retval = *this; ++(*this); return retval; }
                    JMAN_FORCE_INLINE iterator& operator--() noexcept { m_ptr = m_ptr->prev; return *this; }
                    JMAN_FORCE_INLINE iterator operator--(int) noexcept { iterator retval = *this; --(*this); return retval; }

                    JMAN_FORCE_INLINE pointer operator->() const { return __ptr(m_ptr); }
                    JMAN_FORCE_INLINE reference operator*() const { return __value(m_ptr); }

                    JMAN_FORCE_INLINE bool operator==(const iterator& other) const noexcept { return m_ptr == other.m_ptr; }
                    JMAN_FORCE_INLINE bool operator!=(const iterator& other) const noexcept { return m_ptr != other.m_ptr; }

                    operator const_iterator() const { return const_iterator{m_ptr}; };
            };

            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            // --------------------------------- constructors

            linked_list() noexcept { m_dummy.next = m_dummy.prev = &m_dummy; };
            explicit linked_list(size_type size);
            linked_list(size_type size, const_reference value);
            linked_list(std::initializer_list<value_type> list);
            
            linked_list(const linked_list& other);
            linked_list& operator=(const linked_list& other);
            
            linked_list(linked_list&& other) noexcept;
            linked_list& operator=(linked_list&& other) noexcept;

            ~linked_list() noexcept;

            // ------------------------------ accessing

            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference         front() { return __value(m_dummy.next); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference   front() const { return __value(m_dummy.next); }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference         back() { return __value(m_dummy.prev); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference   back() const { return __value(m_dummy.prev); }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference         at(size_type index);
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference   at(size_type index) const;

            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator          find(const_reference value) noexcept;
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator    find(const_reference value) const noexcept; 

            // ---------------------------- iterators

            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator          begin() noexcept { return iterator{m_dummy.next}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator          end() noexcept { return iterator{&m_dummy}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator    begin() const noexcept { return const_iterator{m_dummy.next}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator    end() const noexcept { return const_iterator{&m_dummy}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator    cbegin() const noexcept { return const_iterator{m_dummy.next}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator    cend() const noexcept { return const_iterator{&m_dummy}; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE reverse_iterator          rbegin() noexcept { return reverse_iterator{iterator{&m_dummy}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reverse_iterator          rend() noexcept { return reverse_iterator{iterator{m_dummy.next}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    rbegin() const noexcept { return const_reverse_iterator{const_iterator{m_dummy.next}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    rend() const noexcept { return const_reverse_iterator{const_iterator{&m_dummy}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    crbegin() const noexcept { return const_reverse_iterator{const_iterator{m_dummy.next}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    crend() const noexcept { return const_reverse_iterator{const_iterator{&m_dummy}}; }

            // -------------------------------- stats

            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool              empty() const noexcept { return m_size == 0; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type         size() const noexcept { return m_size; }
            /** @warning: this method is just an aproximation */
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type         max_size() const noexcept { return std::numeric_limits<size_type>::max() / sizeof(__list_node); }

            // -------------------------------- modifiers

            void                                                clear() noexcept;
            void                                                resize(size_type count);
            void                                                resize(size_type count, const_reference value);
            void                                                assign(size_type count, const_reference value);
            /** Uses mergesort for sorting the elements in the linked list */
            ///@{
            void                                                sort();
            template <class Compare> 
            void                                                sort(Compare comp);
            ///@}
            size_type                                           unique();
            template <class BinaryPredicate> 
            size_type                                           unique(BinaryPredicate predicate);
            void                                                splice(const_iterator position, const linked_list& x);
            void                                                splice(const_iterator position, linked_list&& x) noexcept;
            void                                                reverse() noexcept;
            template <class Predicate>
            void                                                remove_if(Predicate pred);
            template <class Predicate> 
            void                                                retain(Predicate);

            // --------------------------------- elements

            template <typename TT>
            requires std::is_convertible_v<TT, T>
            void                                                push_front(TT&& value);
            template <typename TT>
            requires std::is_convertible_v<TT, T>
            void                                                push_back(TT&& value);


            template <typename ...Args>
            requires std::is_constructible_v<T, Args...>
            void                                                emplace_front(Args&&... args);
            template <typename ...Args>
            requires std::is_constructible_v<T, Args...>
            void                                                emplace_back(Args&&... args);

            void pop_front();
            void pop_back();

            template <typename TT>
            requires std::is_convertible_v<TT, T>
            iterator                                            insert(size_type index, TT&& value); 
            template <typename ...Args>
            requires std::is_constructible_v<T, Args...>
            iterator                                            emplace(size_type index, Args&&... args); 


            template <typename TT>
            requires std::is_convertible_v<TT, T>
            iterator                                            insert(iterator it, TT&& value); 
            template <typename ...Args>
            requires std::is_constructible_v<T, Args...>
            iterator                                            emplace(iterator it, Args&&... args); 


            void                                                erase(iterator it); 
            void                                                erase(size_type index); 
    };
}

// ####################################################################################################################
// ----------------------------------------------- PRIVATE CLASS METHODS --------------------------------------------//
// ####################################################################################################################

/**
    @brief 
        cleans nodes backwards until finding the dummy node
*/
template <typename T>
void jman::linked_list<T>::__clean_nodes_up_to_backwards(__list_node_base* node) noexcept
{
    while (node != &m_dummy) {
        __list_node* ptr = static_cast<__list_node*>(node);
        __list_node_base* prev = node->prev;

        jman::destruct_at<T>(__ptr(ptr));
        ::operator delete(static_cast<void*>(node), std::align_val_t{alignof(__list_node)});

        node = prev;
    }
}

/**
    @brief 
        cleans nodes forward until finding the dummy node
*/
template <typename T>
void jman::linked_list<T>::__clean_nodes_up_to_forward(__list_node_base* node) noexcept
{
    while (node != &m_dummy) {
        __list_node* ptr = static_cast<__list_node*>(node);
        __list_node_base* next = node->next;
        
        jman::destruct_at<T>(__ptr(ptr));
        ::operator delete(static_cast<void*>(node), std::align_val_t{alignof(__list_node)});
        
        node = next;
    }
}

template <typename T>
void jman::linked_list<T>::__swap_nodes(__list_node_base* a, __list_node_base* b) noexcept
{
    // Case 1: Nodes are adjacent (a's next neighbor node is b)
    if (a->next == b) {
        __list_node_base* aPrev = a->prev;
        __list_node_base* bNext = b->next;

        aPrev->next = b;
        bNext->prev = a;

        b->prev = aPrev;
        b->next = a;
        a->prev = b;
        a->next = bNext;
    }
    // Case 2: Nodes are adjacent (b's next neighbor node is a)
    else if (b->next == a) {
        __list_node_base* bPrev = b->prev;
        __list_node_base* aNext = a->next;

        bPrev->next = a;
        aNext->prev = b;

        a->prev = bPrev;
        a->next = b;
        b->prev = a;
        b->next = aNext;
    }
    // Case 3: Nodes are not adjacent
    else {
        __list_node_base* bNext = b->next;
        __list_node_base* bPrev = b->prev;

        a->next->prev = b;
        a->prev->next = b;
        b->next = a->next;
        b->prev = a->prev;

        bNext->prev = a;
        bPrev->next = a;
        a->next = bNext;
        a->prev = bPrev;
    }
}

template <typename T>
jman::linked_list<T>::__list_node_base* jman::linked_list<T>::__split() noexcept
{
    size_type half = (m_size - 1) >> 1;
    __list_node_base* current = m_dummy.next;

    while (half > 0) {
        current = current->next;
        --half;
    }

    return current;
}

template <typename T>
template <typename Comp>
jman::linked_list<T>::__list_node_base* jman::linked_list<T>::__sort_merge(__list_node_base* left, __list_node_base* right, Comp comp)
{
    __list_node_base dummy_anchor; // local anchor for the current sub array merge
    __list_node_base* tail = &dummy_anchor;

    while (left && right) {
        if (comp(__value(left), __value(right))) {
            tail->next = left;
            left->prev = tail;
            left = left->next;
        } else {
            tail->next = right;
            right->prev = tail;
            right = right->next;
        }
        tail = tail->next;
    }

    if (left) {
        tail->next = left;
        left->prev = tail;
    } else if (right) {
        tail->next = right;
            right->prev = tail;
    }

    __list_node_base* mergedHead = dummy_anchor.next;
    if (mergedHead) mergedHead->prev = nullptr;
    return mergedHead;
}

// ####################################################################################################################
// --------------------------------------------- CONSTRUCTORS / DESTRUCTORS -----------------------------------------//
// ####################################################################################################################


template <typename T>
jman::linked_list<T>::linked_list(size_type size)
{
    m_dummy.next = &m_dummy;
    m_dummy.prev = &m_dummy;
    if (size == 0) return;

    __list_node_base* current = &m_dummy;

    /*
        guard for when any of the node allocations fails or when T constructor fails in one of the node
        lets hope that the compiler is smart enough to treat this object as unecessary and remove it completly
        thanks to the constexpr if branch
    */
    auto guard = scope_guard([&] {
        if constexpr (!std::is_nothrow_default_constructible_v<T>) {
            __clean_nodes_up_to_backwards(current);

            m_dummy.next = &m_dummy;
            m_dummy.prev = &m_dummy;
        }
    });

    for (size_type i = 0; i < size; ++i)
    {
        __list_node* node = static_cast<__list_node*>(::operator new(sizeof(__list_node), std::align_val_t{alignof(__list_node)}));
        __list_node_base* base_ptr = static_cast<__list_node_base*>(node);

        if constexpr (!std::is_nothrow_default_constructible_v<T>) {
            // wrap it around a try-catch block, ive heard it has zero-cost overhead entering a try-catch block anywhere and the only overhead is the stack unwinding
            // so its cheap
            JMAN_TRY {
                jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer));
            } JMAN_CATCH (...) {
                ::operator delete(static_cast<void*>(node), std::align_val_t{alignof(__list_node)});
                JMAN_RETHROW;
            }
        } else {
            // safe to construct since its marked as nothrow
            jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer));
        }

        base_ptr->prev = current;
        current->next = base_ptr;
        current = base_ptr;
    }
    current->next = &m_dummy;
    m_dummy.prev = current;

    m_size = size;
    guard.dismiss(); 
}

template <typename T>
jman::linked_list<T>::linked_list(size_type size, const_reference value)
{
    m_dummy.next = &m_dummy;
    m_dummy.prev = &m_dummy;
    if (size == 0) return;

    __list_node_base* current = &m_dummy;

    /*
        guard for when any of the node allocations fails or when T constructor fails in one of the node
        lets hope that the compiler is smart enough to treat this object as unecessary and remove it completly
        thanks to the constexpr if branch
    */
    auto guard = scope_guard([&] {
        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            __clean_nodes_up_to_backwards(current);

            m_dummy.next = &m_dummy;
            m_dummy.prev = &m_dummy;
        }
    });

    for (size_type i = 0; i < size; ++i)
    {
        __list_node* node = static_cast<__list_node*>(::operator new(sizeof(__list_node), std::align_val_t{alignof(__list_node)}));
        __list_node_base* base_ptr = static_cast<__list_node_base*>(node);

        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            // wrap it around a try-catch block, ive heard it has zero-cost overhead entering a try-catch block anywhere and the only overhead is the stack unwinding
            // so its cheap
            JMAN_TRY {
                jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), value);
            } JMAN_CATCH (...) {
                ::operator delete(static_cast<void*>(node), std::align_val_t{alignof(__list_node)});
                JMAN_RETHROW;
            }
        } else {
            // safe to construct since its marked as nothrow
            jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), value);
        }

        base_ptr->prev = current;
        current->next = base_ptr;
        current = base_ptr;
    }
    current->next = &m_dummy;
    m_dummy.prev = current;

    m_size = size;
    guard.dismiss(); 
}

template <typename T>
jman::linked_list<T>::linked_list(std::initializer_list<T> list)
{
    m_dummy.next = &m_dummy;
    m_dummy.prev = &m_dummy;
    if (list.size() == 0) return;

    auto copy_iter = list.begin();
    __list_node_base* current = &m_dummy;
    
    /*
        guard for when any of the node allocations fails or when T constructor fails in one of the node
        lets hope that the compiler is smart enough to treat this object as unecessary and remove it completly
        thanks to the constexpr if branch
    */
    auto guard = scope_guard([&] {
        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            __clean_nodes_up_to_backwards(current);

            m_dummy.next = &m_dummy;
            m_dummy.prev = &m_dummy;
        }
    });

    for (size_type i = 0; i < list.size(); ++i)
    {
        __list_node* node = static_cast<__list_node*>(::operator new(sizeof(__list_node), std::align_val_t{alignof(__list_node)}));
        __list_node_base* base_ptr = static_cast<__list_node_base*>(node);

        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            // wrap it around a try-catch block, ive heard it has zero-cost overhead entering a try-catch block anywhere and the only overhead is the stack unwinding
            // so its cheap
            JMAN_TRY {
                jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), copy_iter[i]);
            } JMAN_CATCH (...) {
                ::operator delete(static_cast<void*>(node), std::align_val_t{alignof(__list_node)});
                JMAN_RETHROW;
            }
        } else {
            // safe to construct since its marked as nothrow
            jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), copy_iter[i]);
        }

        base_ptr->prev = current;
        current->next = base_ptr;
        current = base_ptr;
    }
    current->next = &m_dummy;
    m_dummy.prev = current;

    m_size = list.size();
    guard.dismiss(); 
}


template <typename T>
jman::linked_list<T>::linked_list(const linked_list& other)
{
    m_dummy.next = &m_dummy;
    m_dummy.prev = &m_dummy;
    if (other.m_size == 0) return;

    __list_node_base* current = &m_dummy;
    __list_node_base* to_copy = other.m_dummy.next;

    /*
        guard for when any of the node allocations fails or when T constructor fails in one of the node
        lets hope that the compiler is smart enough to treat this object as unecessary and remove it completly
        thanks to the constexpr if branch
    */
    auto guard = scope_guard([&] {
        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            __clean_nodes_up_to_backwards(current);

            m_dummy.next = &m_dummy;
            m_dummy.prev = &m_dummy;
        }
    });

    for (size_type i = 0; i < other.m_size; ++i)
    {
        __list_node* node = static_cast<__list_node*>(::operator new(sizeof(__list_node), std::align_val_t{alignof(__list_node)}));
        __list_node_base* base_ptr = static_cast<__list_node_base*>(node);

        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            // wrap it around a try-catch block, ive heard it has zero-cost overhead entering a try-catch block anywhere and the only overhead is the stack unwinding
            // so its cheap
            JMAN_TRY {
                jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), std::as_const(__value(to_copy)));
            } JMAN_CATCH (...) {
                ::operator delete(static_cast<void*>(node), std::align_val_t{alignof(__list_node)});
                JMAN_RETHROW;
            }
        } else {
            // safe to construct since its marked as nothrow
            jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), std::as_const(__value(to_copy)));
        }
        to_copy = to_copy->next;

        base_ptr->prev = current;
        current->next = base_ptr;
        current = base_ptr;
        
    }
    current->next = &m_dummy;
    m_dummy.prev = current;

    m_size = other.m_size;
    guard.dismiss(); 
}

template <typename T> 
jman::linked_list<T>& jman::linked_list<T>::operator=(const linked_list& other)
{
    if (this != &other)
    {
        const __list_node_base* to_copy = other.m_dummy.next;
        __list_node_base* current = &m_dummy;
        __list_node_base* to_delete = m_dummy.next;
        size_type constructed = 0;

        auto guard = scope_guard([&] {
            if constexpr (
                std::is_nothrow_copy_constructible_v<T> 
                && std::is_nothrow_copy_assignable_v<T> 
            ) {
                if (constructed > m_size) { // the failed node was one of the nodes that wasnt reused, so its safe to delete
                     __list_node_base* prev = current->prev;
                    ::operator delete(static_cast<void*>(current), std::align_val_t{alignof(__list_node)});

                    prev->next = &m_dummy;
                    m_dummy.prev = prev;
                    m_size = constructed; 
                }
            }
        });

        for (size_type i = 0; i < m_size; ++i)
        {
            if (i < other.m_size) {
                // reuse current node
                current = current->next; 
                /*
                    this should be put first to avoid an error where "current" points to invalid/deleted memory
                    making the program go UB, by doing this
                */

                __value(current) = __value(to_copy);

                to_delete = current->next;
                to_copy = to_copy->next;
                
                ++constructed;
            } else {
                // delete node and truncate
                __list_node_base* next = to_delete->next;
                
                jman::destruct_at<T>(__ptr(to_delete));
                ::operator delete(static_cast<void*>(to_delete), std::align_val_t{alignof(__list_node)});

                to_delete = next;
            }
        }

        size_type diff = other.m_size - m_size;
        if (diff > other.m_size) diff = 0; // overflow

        for (size_type i = 0; i < diff; ++i)
        {
            // create new node
            __list_node* node = static_cast<__list_node*>(::operator new(sizeof(__list_node), std::align_val_t{alignof(__list_node)}));
            __list_node_base* base_ptr = static_cast<__list_node_base*>(node);

            if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
                // wrap it around a try-catch block, ive heard it has zero-cost overhead entering a try-catch block anywhere and the only overhead is the stack unwinding
                // so its cheap
                JMAN_TRY {
                    jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), __value(to_copy));
                } JMAN_CATCH (...) {
                    ::operator delete(static_cast<void*>(node), std::align_val_t{alignof(__list_node)});
                    JMAN_RETHROW;
                }
            } else {
                // safe to construct since its marked as nothrow
                jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), __value(to_copy));
            }

            base_ptr->prev = current;
            current->next = base_ptr;
            current = base_ptr;
            
            to_copy = to_copy->next; 
            ++constructed;
        }

        current->next = &m_dummy;
        m_dummy.prev = current;

        m_size = other.m_size;
        guard.dismiss(); 
    }

    return *this;
}

template <typename T>
jman::linked_list<T>::linked_list(linked_list&& other) noexcept
{
    m_size = other.m_size;

    m_dummy.next = other.m_dummy.next;
    m_dummy.prev = other.m_dummy.prev;
    m_dummy.next->prev = &m_dummy;
    m_dummy.prev->next = &m_dummy;

    other.m_size = 0;
    other.m_dummy.next = other.m_dummy.prev = &other.m_dummy;
}
template <typename T>
jman::linked_list<T>& jman::linked_list<T>::operator=(linked_list&& other) noexcept
{
    if (this != &other)
    {
        clear();

        m_size = other.m_size;

        m_dummy.next = other.m_dummy.next;
        m_dummy.prev = other.m_dummy.prev;
        m_dummy.next->prev = &m_dummy;
        m_dummy.prev->next = &m_dummy;

        other.m_size = 0;
        other.m_dummy.next = other.m_dummy.prev = &other.m_dummy;
    }

    return *this;
}
template <typename T>
jman::linked_list<T>::~linked_list() noexcept
{
    clear();
}

// ####################################################################################################################
// ------------------------------------------------ ACCESSING ELEMENTS ----------------------------------------------//
// ####################################################################################################################

template <typename T>
jman::linked_list<T>::reference jman::linked_list<T>::at(size_type index)
{   
    if (index >= m_size) JMAN_THROW(std::out_of_range("jman::linked_list<T>::at() failed: index provided out of range."));

    size_type half = (m_size - 1) >> 1;
    if (index < half) // start from the head
    {
        __list_node_base* current = m_dummy.next;

        while (index > 0) {
            current = current->next;
            --index;
        }

        return __value(current);
    } else { // start from the tail
        __list_node_base* current = m_dummy.prev;

        while (index < (m_size - 1)) {
            current = current->prev;
            ++index;
        }

        return __value(current);
    }
}
template <typename T>
jman::linked_list<T>::const_reference jman::linked_list<T>::at(size_type index) const
{
    if (index >= m_size) JMAN_THROW(std::out_of_range("jman::linked_list<T>::at() failed: index provided out of range."));

    size_type half = (m_size - 1) >> 1;
    if (index < half) // start from the head
    {
        __list_node_base* current = m_dummy.next;

        while (index > 0) {
            current = current->next;
            --index;
        }

        return __value(current);
    } else { // start from the tail
        __list_node_base* current = m_dummy.prev;

        while (index < (m_size - 1)) {
            current = current->prev;
            ++index;
        }

        return __value(current);
    }
}

template <typename T>
jman::linked_list<T>::iterator jman::linked_list<T>::find(const T& value) noexcept
{
    if (empty()) return end();

    __list_node_base* current = m_dummy.next;
    size_type counter = 0;

    while (current != &m_dummy) {
        if (__value(current) == value) return iterator{current};
        current = current->next;
    }

    return end();
}
template <typename T>
jman::linked_list<T>::const_iterator jman::linked_list<T>::find(const T& value) const noexcept
{
    if (empty()) return end();

    __list_node_base* current = m_dummy.next;

    while (current != &m_dummy) {
        if (__value(current) == value) return iterator{current};
        current = current->next;
    }

    return end();
}

// ####################################################################################################################
// -------------------------------------------------- RANGE MODIFIERS -----------------------------------------------//
// ####################################################################################################################

template <typename T>
void jman::linked_list<T>::clear() noexcept
{
    __clean_nodes_up_to_forward(m_dummy.next);

    m_dummy.next = m_dummy.prev = &m_dummy;
    m_size = 0;
}
template <typename T>
void jman::linked_list<T>::resize(size_type count)
{
    if (count > m_size) {
        // increase
        __list_node_base* current = &m_dummy;
        __list_node_base* head = nullptr;

        auto guard = scope_guard([&] {
            if constexpr (!std::is_nothrow_default_constructible_v<T>)
                __clean_nodes_up_to_backwards(current);
        });

        for (size_type i = m_size; i < count; ++i)
        {
            __list_node* node = static_cast<__list_node*>(::operator new(sizeof(__list_node), std::align_val_t{alignof(__list_node)}));
            __list_node_base* base_ptr = static_cast<__list_node_base*>(node);

            if constexpr (!std::is_nothrow_default_constructible_v<T>) {
                JMAN_TRY {
                    jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer));
                } JMAN_CATCH (...) {
                    ::operator delete(static_cast<void*>(node), std::align_val_t{alignof(__list_node)});
                    JMAN_RETHROW;
                }
            } else {
                // safe to construct since its marked as nothrow
                jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer));
            }

            base_ptr->prev = current;

            if (current != &m_dummy) 
                current->next = base_ptr;
            else
                head = base_ptr;

            current = base_ptr;
        }

        current->next = &m_dummy;
        head->prev = m_dummy.prev; // the head previously pointed to the dummy node for the cleanup guard run fine on the nodes created

        m_dummy.prev->next = head;
        m_dummy.prev = current;

        m_size = count;
        guard.dismiss(); 
    } else if (count < m_size) {
        // truncate
        __list_node_base* current = m_dummy.prev;
        for (size_type i = m_size; i > count; --i)
        {
            __list_node_base* prev = current->prev;
            
            jman::destruct_at<T>(__ptr(current));
            ::operator delete(static_cast<void*>(current), std::align_val_t{alignof(__list_node)});

            current = prev;
        }

        current->next = &m_dummy;
        m_dummy.prev = current;

        m_size = count;
    }
}
template <typename T>
void jman::linked_list<T>::resize(size_type count, const_reference value)
{
    if (count > m_size) {
        // increase
        __list_node_base* current = &m_dummy;
        __list_node_base* head = nullptr;

        auto guard = scope_guard([&] {
            if constexpr (!std::is_nothrow_copy_constructible_v<T>)
                __clean_nodes_up_to_backwards(current);
        });

        for (size_type i = m_size; i < count; ++i)
        {
            __list_node* node = static_cast<__list_node*>(::operator new(sizeof(__list_node), std::align_val_t{alignof(__list_node)}));
            __list_node_base* base_ptr = static_cast<__list_node_base*>(node);

            if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
                JMAN_TRY {
                    jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), value);
                } JMAN_CATCH (...) {
                    ::operator delete(static_cast<void*>(node), std::align_val_t{alignof(__list_node)});
                    JMAN_RETHROW;
                }
            } else {
                // safe to construct since its marked as nothrow
                jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), value);
            }

            base_ptr->prev = current;

            if (current != &m_dummy) 
                current->next = base_ptr;
            else
                head = base_ptr;

            current = base_ptr;
        }

        current->next = &m_dummy;
        head->prev = m_dummy.prev; // the head previously pointed to the dummy node for the cleanup guard run fine on the nodes created

        m_dummy.prev->next = head;
        m_dummy.prev = current;

        m_size = count;
        guard.dismiss(); 
    } else if (count < m_size) {
        // truncate
        __list_node_base* current = m_dummy.prev;
        for (size_type i = m_size; i > count; --i)
        {
            __list_node_base* prev = current->prev;
            
            jman::destruct_at<T>(__ptr(current));
            ::operator delete(static_cast<void*>(current), std::align_val_t{alignof(__list_node)});

            current = prev;
        }

        current->next = &m_dummy;
        m_dummy.prev = current;

        m_size = count;
    }
}

template <typename T>
void jman::linked_list<T>::assign(size_type count, const_reference value)
{
    if (count == 0) return;
    __list_node_base* current = &m_dummy;
    __list_node_base* to_delete = m_dummy.next;
    size_type constructed = 0;

    auto guard = scope_guard([&] {
        if constexpr (
            std::is_nothrow_copy_constructible_v<T> 
            && std::is_nothrow_copy_assignable_v<T> 
        ) {
            if (constructed > m_size) { // the failed node was one of the nodes that wasnt reused, so its safe to delete
                    __list_node_base* prev = current->prev;
                ::operator delete(static_cast<void*>(current), std::align_val_t{alignof(__list_node)});

                prev->next = &m_dummy;
                m_dummy.prev = prev;
            }

            m_size = constructed; 
        }
    });

    for (size_type i = 0; i < m_size; ++i)
    {
        if (i < count) {
            // reuse current node
            current = current->next; 
            /*
                this should be put first to avoid an error where "current" points to invalid/deleted memory
                making the program go UB, by doing this
            */

            __value(current) = value;

            to_delete = current->next;
            
            ++constructed;
        } else {
            // delete node and truncate
            __list_node_base* next = to_delete->next;
            
            jman::destruct_at<T>(__ptr(to_delete));
            ::operator delete(static_cast<void*>(to_delete), std::align_val_t{alignof(__list_node)});

            to_delete = next;
        }
    }

    size_type diff = count - m_size;
    if (diff > count) diff = 0; // overflow

    for (size_type i = 0; i < diff; ++i)
    {
        // create new node
        __list_node* node = static_cast<__list_node*>(::operator new(sizeof(__list_node), std::align_val_t{alignof(__list_node)}));
        __list_node_base* base_ptr = static_cast<__list_node_base*>(node);

        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            // wrap it around a try-catch block, ive heard it has zero-cost overhead entering a try-catch block anywhere and the only overhead is the stack unwinding
            // so its cheap
            JMAN_TRY {
                jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), value);
            } JMAN_CATCH (...) {
                ::operator delete(static_cast<void*>(node), std::align_val_t{alignof(__list_node)});
                JMAN_RETHROW;
            }
        } else {
            // safe to construct since its marked as nothrow
            jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), value);
        }

        base_ptr->prev = current;
        current->next = base_ptr;
        current = base_ptr;
        
        ++constructed;
    }

    current->next = &m_dummy;
    m_dummy.prev = current;

    m_size = count;
    guard.dismiss(); 
}
template <typename T>
void jman::linked_list<T>::sort()
{
    sort(std::less{});
}
template <typename T>
template <typename Compare> 
void jman::linked_list<T>::sort(Compare comp)
{
    if (m_size < 2) return;

    // make the head and tail nodes point to nullptr to make the sorting process easier
    __list_node_base* flatHead = m_dummy.next;
    m_dummy.prev->next = nullptr;
    flatHead->prev = nullptr; 

    for (size_type step = 1; step < m_size; step <<= 1) {
        __list_node_base* curr = flatHead;
        __list_node_base* prevTail = nullptr;
        flatHead = nullptr;

        while (curr) {
            __list_node_base* left = curr;
            __list_node_base* leftTail = nullptr;
            size_type leftSize = 0;
            
            // grab the left sub list tail
            while (curr && leftSize < step) {
                leftTail = curr; 
                curr = curr->next;
                ++leftSize;
            }

            // if there is no right to merge with this left, merge it with the current already sorted list
            if (!curr) {
                if (prevTail) {
                    prevTail->next = left;
                    left->prev = prevTail;
                } else {
                    flatHead = left;
                }
                break;
            }

            // disconnects the left with the current right
            if (leftTail) leftTail->next = nullptr;

            __list_node_base* right = curr;
            __list_node_base* rightTail = nullptr;
            size_type rightSize = 0;
            
            // grab the tail for the right node
            while (curr && rightSize < step) {
                rightTail = curr;
                curr = curr->next;
                rightSize++;
            }

            // disconnects the right with the rest of the list 
            if (rightTail) rightTail->next = nullptr;

            __list_node_base* mergedHead = __sort_merge(left, right, comp);

            // links the recent merged with the already sorted section
            if (prevTail) {
                prevTail->next = mergedHead;
                mergedHead->prev = prevTail;
            } else {
                flatHead = mergedHead;
            }

            __list_node_base* mergedTail = mergedHead;
            while (mergedTail->next)
                mergedTail = mergedTail->next;
            prevTail = mergedTail;
        }
    }

    // reconnect the sorted nodes back to m_dummy
    m_dummy.next = flatHead;
    flatHead->prev = &m_dummy;

    // walk to the last real element
    __list_node_base* finalTail = flatHead;
    while (finalTail->next) 
        finalTail = finalTail->next;
    finalTail->next = &m_dummy;
    m_dummy.prev = finalTail;
}
template <typename T>
jman::linked_list<T>::size_type jman::linked_list<T>::unique()
{
    if (m_size < 2) return 0;
    __list_node_base* current = m_dummy.next;
    size_type removed = 0;

    while (current->next != &m_dummy) {
        __list_node_base* next = current->next;

        if (__value(next) == __value(current))
        {
            jman::destruct_at(__ptr(next));

            next->next->prev = current;
            current->next = next->next;

            ::operator delete(static_cast<void*>(next), std::align_val_t{alignof(__list_node)});
            --m_size;

            next = current;
            ++removed;
        }

        current = next;
    }

    return removed;
}
template <typename T>
template <class BinaryPredicate> 
jman::linked_list<T>::size_type jman::linked_list<T>::unique(BinaryPredicate predicate)
{
    if (m_size < 2) return 0;
    __list_node_base* current = m_dummy.next;
    size_type removed = 0;

    while (current->next != &m_dummy) {
        __list_node_base* next = current->next;

        if (predicate(current, next))
        {
            jman::destruct_at(__ptr(next));

            next->next->prev = current;
            current->next = next->next;

            ::operator delete(static_cast<void*>(next), std::align_val_t{alignof(__list_node)});
            --m_size;

            next = current;
            ++removed;
        }

        current = next;
    }

    return removed;
}
template <typename T>
void jman::linked_list<T>::splice(const_iterator position, const linked_list& x)
{
    if (x.m_size == 0 || &x == this) return;

    auto insertAt = const_cast<__list_node_base*>(position.m_ptr);

    __list_node_base* current = &m_dummy; // for safety
    __list_node_base* head = nullptr;
    __list_node_base* to_copy = x.m_dummy.next;

    auto guard = scope_guard([&] {
        if constexpr (!std::is_nothrow_default_constructible_v<T>)
            __clean_nodes_up_to_backwards(current);
    });

    for (size_type i = 0; i < x.m_size; ++i)
    {
        __list_node* node = static_cast<__list_node*>(::operator new(sizeof(__list_node), std::align_val_t{alignof(__list_node)}));
        __list_node_base* base_ptr = static_cast<__list_node_base*>(node);

        if constexpr (!std::is_nothrow_default_constructible_v<T>) {
            JMAN_TRY {
                jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), __value(to_copy));
            } JMAN_CATCH (...) {
                ::operator delete(static_cast<void*>(node), std::align_val_t{alignof(__list_node)});
                JMAN_RETHROW;
            }
        } else {
            // safe to construct since its marked as nothrow
            jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), __value(to_copy));
        }

        base_ptr->prev = current;

        if (current != &m_dummy) 
            current->next = base_ptr;
        else
            head = base_ptr;

        current = base_ptr;
        to_copy = to_copy->next;
    }

    head->prev = insertAt->prev;
    current->next = insertAt;

    insertAt->prev->next = head;

    head->prev = insertAt->prev;
    current->next = insertAt;

    insertAt->prev = current;

    m_size += x.m_size;
    guard.dismiss(); 
}
template <typename T>
void jman::linked_list<T>::splice(const_iterator position, linked_list&& x) noexcept
{
    if (x.m_size == 0 || &x == this) return;

    auto insertAt = const_cast<__list_node_base*>(position.m_ptr);
    insertAt->prev->next = x.m_dummy.next;

    x.m_dummy.next->prev = insertAt->prev;
    x.m_dummy.prev->next = insertAt;

    insertAt->prev = x.m_dummy.prev;

    m_size += x.m_size;
    
    x.m_dummy.next = x.m_dummy.prev = &x.m_dummy;
    x.m_size = 0;
}


template <typename T>
void jman::linked_list<T>::reverse() noexcept
{
    if (m_size < 2) return;

    __list_node_base* l = m_dummy.next;
    __list_node_base* r = m_dummy.prev;

    size_type li = 0;
    size_type ri = m_size - 1;

    while (li < ri) {
        __list_node_base* nextR = r->prev;
        __list_node_base* nextL = l->next;

        __swap_nodes(l, r);
        r = nextR;
        l = nextL;

        ++li;
        --ri;
    }
}

// ####################################################################################################################
// ----------------------------------------------------- ELEMENTS  --------------------------------------------------//
// ####################################################################################################################


template <typename T>
template <typename TT>
requires std::is_convertible_v<TT, T>
void jman::linked_list<T>::push_front(TT&& value)
{
    __list_node* node = static_cast<__list_node*>(::operator new(sizeof(__list_node), std::align_val_t{alignof(__list_node)}));
    __list_node_base* base_ptr = static_cast<__list_node_base*>(node);
    
    jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), static_cast<T>(std::forward<TT>(value)));

    base_ptr->next = m_dummy.next;
    base_ptr->prev = &m_dummy;

    m_dummy.next->prev = base_ptr;
    m_dummy.next = base_ptr;

    ++m_size;
}


template <typename T>
template <typename TT>
requires std::is_convertible_v<TT, T>
void jman::linked_list<T>::push_back(TT&& value)
{
    __list_node* node = static_cast<__list_node*>(::operator new(sizeof(__list_node), std::align_val_t{alignof(__list_node)}));
    __list_node_base* base_ptr = static_cast<__list_node_base*>(node);
    
    jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), static_cast<T>(std::forward<TT>(value)));

    base_ptr->next = &m_dummy;
    base_ptr->prev = m_dummy.prev;

    m_dummy.prev->next = base_ptr;
    m_dummy.prev = base_ptr;

    ++m_size;
}

template <typename T>
template <typename ...Args>
requires std::is_constructible_v<T, Args...>
void jman::linked_list<T>::emplace_front(Args&&... args)
{
    __list_node* node = static_cast<__list_node*>(::operator new(sizeof(__list_node), std::align_val_t{alignof(__list_node)}));
    __list_node_base* base_ptr = static_cast<__list_node_base*>(node);
    
    jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), std::forward<Args>(args)...);

    base_ptr->next = m_dummy.next;
    base_ptr->prev = &m_dummy;

    m_dummy.next->prev = base_ptr;
    m_dummy.next = base_ptr;

    ++m_size;
}
template <typename T>
template <typename ...Args>
requires std::is_constructible_v<T, Args...>
void jman::linked_list<T>::emplace_back(Args&&... args)
{
    __list_node* node = static_cast<__list_node*>(::operator new(sizeof(__list_node), std::align_val_t{alignof(__list_node)}));
    __list_node_base* base_ptr = static_cast<__list_node_base*>(node);
    
    jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), std::forward<Args>(args)...);

    base_ptr->next = &m_dummy;
    base_ptr->prev = m_dummy.prev;

    m_dummy.prev->next = base_ptr;
    m_dummy.prev = base_ptr;

    ++m_size;
}

template <typename T>
void jman::linked_list<T>::pop_front()
{
    if (empty()) return;

    __list_node_base* node = m_dummy.next;
    jman::destruct_at(__ptr(node));

    node->next->prev = node->prev;
    node->prev->next = node->next;

    ::operator delete(static_cast<void*>(node), std::align_val_t{alignof(__list_node)});
    --m_size;
}
template <typename T>
void jman::linked_list<T>::pop_back()
{
    if (empty()) return;

    __list_node_base* node = m_dummy.next;
    jman::destruct_at(__ptr(node));

    node->next->prev = node->prev;
    node->prev->next = node->next;

    ::operator delete(static_cast<void*>(node), std::align_val_t{alignof(__list_node)});
    --m_size;
}

template <typename T>
template <typename TT>
requires std::is_convertible_v<TT, T>
jman::linked_list<T>::iterator jman::linked_list<T>::insert(size_type index, TT&& value)
{
    return insert(at(index), std::forward<TT>(value));
}
template <typename T>
template <typename ...Args>
requires std::is_constructible_v<T, Args...>
jman::linked_list<T>::iterator jman::linked_list<T>::emplace(size_type index, Args&&... args)
{
    return emplace(at(index), std::forward<Args>(args)...);
}

template <typename T>
template <typename TT>
requires std::is_convertible_v<TT, T>
jman::linked_list<T>::iterator jman::linked_list<T>::insert(iterator it, TT&& value)
{
    if (it == end()) return end();
    __list_node* node = static_cast<__list_node*>(::operator new(sizeof(__list_node), std::align_val_t{alignof(__list_node)}));
    __list_node_base* base_ptr = static_cast<__list_node_base*>(node);
    
    jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), static_cast<T>(std::forward<TT>(value)));   
    
    base_ptr->next = it.m_ptr;
    base_ptr->prev = it.m_ptr->prev;

    it.m_ptr->prev->next = base_ptr;
    it.m_ptr->prev = base_ptr;

    ++m_size;

    return iterator{base_ptr};
}
template <typename T>
template <typename ...Args>
requires std::is_constructible_v<T, Args...>
jman::linked_list<T>::iterator jman::linked_list<T>::emplace(iterator it, Args&&... args)
{
    if (it == end()) return end();
    __list_node* node = static_cast<__list_node*>(::operator new(sizeof(__list_node), std::align_val_t{alignof(__list_node)}));
    __list_node_base* base_ptr = static_cast<__list_node_base*>(node);
    
    jman::construct_at<T>(reinterpret_cast<T*>(&node->buffer), std::forward<Args>(args)...);   
    
    base_ptr->next = it.m_ptr;
    base_ptr->prev = it.m_ptr->prev;

    it.m_ptr->prev->next = base_ptr;
    it.m_ptr->prev = base_ptr;

    ++m_size;

    return iterator{base_ptr};
}

template <typename T>
void jman::linked_list<T>::erase(iterator it)
{
    if (empty()) return;
    if (it == end()) return;
    __list_node_base* node = it.m_ptr;
    jman::destruct_at(__ptr(node));

    node->next->prev = node->prev;
    node->prev->next = node->next;

    ::operator delete(static_cast<void*>(node), std::align_val_t{alignof(__list_node)});
    --m_size;
}
template <typename T>
void jman::linked_list<T>::erase(size_type index)
{
    erase(at(index));
}