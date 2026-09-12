#pragma once

#include "justaman/errors/safe_exceptions.hpp"
#include <justaman/memory/lifetime.hpp>
#include <justaman/utils/attributes.h>
#include <justaman/algorithms/numeric.hpp>
#include <justaman/utils/scope.hpp>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <type_traits>


namespace jman::detail 
{
    template <typename T>
    struct linked_list_iterator;
    template <typename T>
    struct linked_list_const_iterator;
} // namespace jman::detail


namespace jman 
{
    /**
        @brief
             my custom implementation of a fast and exception safe doubly circular linked list
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

            using iterator = detail::linked_list_iterator<T>;
            using const_iterator = detail::linked_list_const_iterator<T>;

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

            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference         front() noexcept        { return m_value(m_dummy.next); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference   front() const noexcept  { return m_value(m_dummy.next); }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference         back() noexcept         { return m_value(m_dummy.prev); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference   back() const noexcept   { return m_value(m_dummy.prev); }

            JMAN_NO_DISCARD reference                           at(size_type index);
            JMAN_NO_DISCARD const_reference                     at(size_type index) const;

            JMAN_NO_DISCARD iterator                            find(const_reference value) noexcept;
            JMAN_NO_DISCARD const_iterator                      find(const_reference value) const noexcept; 

            // ---------------------------- iterators

            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator          begin() noexcept            { return iterator{m_dummy.next}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE iterator          end() noexcept              { return iterator{&m_dummy}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator    begin() const noexcept      { return const_iterator{m_dummy.next}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator    end() const noexcept        { return const_iterator{&m_dummy}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator    cbegin() const noexcept     { return const_iterator{m_dummy.next}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_iterator    cend() const noexcept       { return const_iterator{&m_dummy}; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE reverse_iterator          rbegin() noexcept           { return reverse_iterator{iterator{&m_dummy}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reverse_iterator          rend() noexcept             { return reverse_iterator{iterator{m_dummy.next}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    rbegin() const noexcept     { return const_reverse_iterator{const_iterator{m_dummy.next}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    rend() const noexcept       { return const_reverse_iterator{const_iterator{&m_dummy}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    crbegin() const noexcept    { return const_reverse_iterator{const_iterator{m_dummy.next}}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reverse_iterator    crend() const noexcept      { return const_reverse_iterator{const_iterator{&m_dummy}}; }

            // -------------------------------- stats

            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool              empty() const noexcept          { return m_size == 0; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type         size() const noexcept           { return m_size; }
            /** @warning: this method is just an aproximation */
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type         max_size() const noexcept       { return std::numeric_limits<size_type>::max() / sizeof(list_node); }

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

        private:
            struct list_node_base 
            {
                list_node_base* next;
                list_node_base* prev;
            };

            struct list_node: public list_node_base
            {
                alignas(T) std::byte buffer[sizeof(T)];
            };

            list_node_base      m_dummy {};
            std::size_t         m_size = 0;

        private:
            JMAN_NO_DISCARD static JMAN_FORCE_INLINE T*         m_val_ptr(list_node* node) noexcept { return std::launder(reinterpret_cast<T*>(&node->buffer)); }
            JMAN_NO_DISCARD static JMAN_FORCE_INLINE const T*   m_val_ptr(const list_node* node) noexcept { return std::launder(reinterpret_cast<const T*>(&node->buffer)); }

            JMAN_NO_DISCARD static JMAN_FORCE_INLINE T*         m_val_ptr(list_node_base* node) noexcept { return std::launder(reinterpret_cast<T*>(&static_cast<list_node*>(node)->buffer)); }
            JMAN_NO_DISCARD static JMAN_FORCE_INLINE const T*   m_val_ptr(const list_node_base* node) noexcept { return std::launder(reinterpret_cast<const T*>(&static_cast<const list_node*>(node)->buffer)); }

            JMAN_NO_DISCARD static JMAN_FORCE_INLINE T&         m_value(list_node* node) noexcept { return *std::launder(reinterpret_cast<T*>(&node->buffer)); }
            JMAN_NO_DISCARD static JMAN_FORCE_INLINE const T&   m_value(const list_node* node) noexcept { return *std::launder(reinterpret_cast<const T*>(&node->buffer)); }

            JMAN_NO_DISCARD static JMAN_FORCE_INLINE T&         m_value(list_node_base* node) noexcept { return *std::launder(reinterpret_cast<T*>(&static_cast<list_node*>(node)->buffer)); }
            JMAN_NO_DISCARD static JMAN_FORCE_INLINE const T&   m_value(const list_node_base* node) noexcept { return *std::launder(reinterpret_cast<const T*>(&static_cast<const list_node*>(node)->buffer)); }

            JMAN_NO_DISCARD static JMAN_FORCE_INLINE linked_list<T>::list_node*     m_alloc_node();
            static JMAN_FORCE_INLINE void                                           m_dealloc_node(void* node) noexcept;

            void                        m_clean_nodes_up_to_backwards(list_node_base* node) noexcept;
            void                        m_clean_nodes_up_to_forward(list_node_base* node) noexcept;

            void                        m_swap_nodes(list_node_base* a, list_node_base* b) noexcept;

            template <typename Comp>
            list_node_base*             m_sort_merge(list_node_base* first, list_node_base* second, Comp comp);
    };
} // namespace jman

// #####################################################################################################################
// --------------------------------------------------- ITERATORS IMPL ------------------------------------------------//
// #####################################################################################################################

namespace jman::detail 
{
    template <typename T>
    struct linked_list_const_iterator
    {
        private: 
            using node = linked_list<T>::list_node_base;

            friend class linked_list<T>;
            const node* m_val_ptr;
        
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type        = T;
            using difference_type   = std::ptrdiff_t;
            using reference         = const T&;
            using pointer           = const T*;

            explicit linked_list_const_iterator(node* ptr) noexcept
                : m_val_ptr(ptr) {}

            JMAN_FORCE_INLINE linked_list_const_iterator& operator++() noexcept { m_val_ptr = m_val_ptr->next; return *this; }
            JMAN_FORCE_INLINE linked_list_const_iterator operator++(int) noexcept { linked_list_const_iterator retval = *this; ++(*this); return retval; }
            JMAN_FORCE_INLINE linked_list_const_iterator& operator--() noexcept { m_val_ptr = m_val_ptr->prev; return *this; }
            JMAN_FORCE_INLINE linked_list_const_iterator operator--(int) noexcept { linked_list_const_iterator retval = *this; --(*this); return retval; }

            JMAN_FORCE_INLINE pointer operator->() const { return m_val_ptr(m_val_ptr); }
            JMAN_FORCE_INLINE reference operator*() const { return m_value(m_val_ptr); }

            JMAN_FORCE_INLINE bool operator==(const linked_list_const_iterator& other) const noexcept { return m_val_ptr == other.m_val_ptr; }
            JMAN_FORCE_INLINE bool operator!=(const linked_list_const_iterator& other) const noexcept { return m_val_ptr != other.m_val_ptr; }
    };

    template <typename T>
    struct linked_list_iterator
    {
        private: 
            using node = linked_list<T>::list_node_base;

            friend class linked_list<T>;
            node* m_val_ptr;
        
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type        = T;
            using difference_type   = std::ptrdiff_t;
            using reference         = T&;
            using pointer           = T*;

            explicit linked_list_iterator(node* ptr) noexcept
                : m_val_ptr(ptr) {}
            
            JMAN_FORCE_INLINE linked_list_iterator& operator++() noexcept { m_val_ptr = m_val_ptr->next; return *this; }
            JMAN_FORCE_INLINE linked_list_iterator operator++(int) noexcept { linked_list_iterator retval = *this; ++(*this); return retval; }
            JMAN_FORCE_INLINE linked_list_iterator& operator--() noexcept { m_val_ptr = m_val_ptr->prev; return *this; }
            JMAN_FORCE_INLINE linked_list_iterator operator--(int) noexcept { linked_list_iterator retval = *this; --(*this); return retval; }

            JMAN_FORCE_INLINE pointer operator->() const { return m_val_ptr(m_val_ptr); }
            JMAN_FORCE_INLINE reference operator*() const { return m_value(m_val_ptr); }

            JMAN_FORCE_INLINE bool operator==(const linked_list_iterator& other) const noexcept { return m_val_ptr == other.m_val_ptr; }
            JMAN_FORCE_INLINE bool operator!=(const linked_list_iterator& other) const noexcept { return m_val_ptr != other.m_val_ptr; }

            operator linked_list_const_iterator<T>() const { return linked_list_const_iterator{m_val_ptr}; };
    };
} // jman::detail

// ####################################################################################################################
// ----------------------------------------------- PRIVATE CLASS METHODS --------------------------------------------//
// ####################################################################################################################

namespace jman {

template <typename T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE linked_list<T>::list_node* linked_list<T>::m_alloc_node()
{
    return static_cast<list_node*>(::operator new(sizeof(list_node), std::align_val_t{alignof(list_node)}));
}
template <typename T>
JMAN_FORCE_INLINE void linked_list<T>::m_dealloc_node(void* node) noexcept
{
    ::operator delete(node, std::align_val_t{alignof(list_node)});
}




template <typename T>
void linked_list<T>::m_clean_nodes_up_to_backwards(list_node_base* node) noexcept
{
    while (node != &m_dummy) {
        list_node* ptr = static_cast<list_node*>(node);
        list_node_base* prev = node->prev;

        jman::destruct_at<T>(m_val_ptr(ptr));
        m_dealloc_node(node);

        node = prev;
    }
}

template <typename T>
void linked_list<T>::m_clean_nodes_up_to_forward(list_node_base* node) noexcept
{
    while (node != &m_dummy) {
        list_node* ptr = static_cast<list_node*>(node);
        list_node_base* next = node->next;
        
        jman::destruct_at<T>(m_val_ptr(ptr));
        m_dealloc_node(node);
        
        node = next;
    }
}





template <typename T>
void linked_list<T>::m_swap_nodes(list_node_base* a, list_node_base* b) noexcept
{
    // Case 1: Nodes are adjacent (a's next neighbor node is b)
    if (a->next == b) {
        list_node_base* aPrev = a->prev;
        list_node_base* bNext = b->next;

        aPrev->next = b;
        bNext->prev = a;

        b->prev = aPrev;
        b->next = a;
        a->prev = b;
        a->next = bNext;
    }
    // Case 2: Nodes are adjacent (b's next neighbor node is a)
    else if (b->next == a) {
        list_node_base* bPrev = b->prev;
        list_node_base* aNext = a->next;

        bPrev->next = a;
        aNext->prev = b;

        a->prev = bPrev;
        a->next = b;
        b->prev = a;
        b->next = aNext;
    }
    // Case 3: Nodes are not adjacent
    else {
        list_node_base* bNext = b->next;
        list_node_base* bPrev = b->prev;

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
template <typename Comp>
linked_list<T>::list_node_base* linked_list<T>::m_sort_merge(list_node_base* left, list_node_base* right, Comp comp)
{
    list_node_base dummy_anchor; // local anchor for the current sub array merge
    list_node_base* tail = &dummy_anchor;

    while (left && right) {
        if (comp(m_value(left), m_value(right))) {
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

    list_node_base* mergedHead = dummy_anchor.next;
    if (mergedHead) mergedHead->prev = nullptr;
    return mergedHead;
}

// ####################################################################################################################
// --------------------------------------------- CONSTRUCTORS / DESTRUCTORS -----------------------------------------//
// ####################################################################################################################


template <typename T>
linked_list<T>::linked_list(size_type size)
{
    m_dummy.next = &m_dummy;
    m_dummy.prev = &m_dummy;
    if (size == 0) return;

    list_node_base* current = &m_dummy;

    auto guard = jman::scope_guard([&] {
        if constexpr (!std::is_nothrow_default_constructible_v<T>) {
            m_clean_nodes_up_to_backwards(current);

            m_dummy.next = &m_dummy;
            m_dummy.prev = &m_dummy;
        }
    });

    for (size_type i = 0; i < size; ++i)
    {
        list_node* node = m_alloc_node();
        list_node_base* base_ptr = static_cast<list_node_base*>(node);

        if constexpr (!std::is_nothrow_default_constructible_v<T>) {
            // wrap it around a try-catch block, ive heard it has zero-cost overhead entering a try-catch block anywhere and the only overhead is the stack unwinding
            // so its cheap
            JMAN_TRY {
                jman::construct_at<T>(m_val_ptr(node));
            } JMAN_CATCH (...) {
                m_dealloc_node(node);
                JMAN_RETHROW;
            }
        } else {
            // safe to construct since its marked as nothrow
            jman::construct_at<T>(m_val_ptr(node));
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
linked_list<T>::linked_list(size_type size, const_reference value)
{
    m_dummy.next = &m_dummy;
    m_dummy.prev = &m_dummy;
    if (size == 0) return;

    list_node_base* current = &m_dummy;

    auto guard = jman::scope_guard([&] {
        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            m_clean_nodes_up_to_backwards(current);

            m_dummy.next = &m_dummy;
            m_dummy.prev = &m_dummy;
        }
    });

    for (size_type i = 0; i < size; ++i)
    {
        list_node* node = m_alloc_node();
        list_node_base* base_ptr = static_cast<list_node_base*>(node);

        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            // wrap it around a try-catch block, ive heard it has zero-cost overhead entering a try-catch block anywhere and the only overhead is the stack unwinding
            // so its cheap
            JMAN_TRY {
                jman::construct_at<T>(m_val_ptr(node), value);
            } JMAN_CATCH (...) {
                m_dealloc_node(node);
                JMAN_RETHROW;
            }
        } else {
            // safe to construct since its marked as nothrow
            jman::construct_at<T>(m_val_ptr(node), value);
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
linked_list<T>::linked_list(std::initializer_list<T> list)
{
    m_dummy.next = &m_dummy;
    m_dummy.prev = &m_dummy;
    if (list.size() == 0) return;

    list_node_base* current = &m_dummy;
    
    auto guard = jman::scope_guard([&] {
        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            m_clean_nodes_up_to_backwards(current);

            m_dummy.next = &m_dummy;
            m_dummy.prev = &m_dummy;
        }
    });

    for (size_type i = 0; i < list.size(); ++i)
    {
        list_node* node = m_alloc_node();
        list_node_base* base_ptr = static_cast<list_node_base*>(node);

        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            // wrap it around a try-catch block, ive heard it has zero-cost overhead entering a try-catch block anywhere and the only overhead is the stack unwinding
            // so its cheap
            JMAN_TRY {
                jman::construct_at<T>(m_val_ptr(node), list[i]);
            } JMAN_CATCH (...) {
                m_dealloc_node(node);
                JMAN_RETHROW;
            }
        } else {
            // safe to construct since its marked as nothrow
            jman::construct_at<T>(m_val_ptr(node), list[i]);
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
linked_list<T>::linked_list(const linked_list& other)
{
    m_dummy.next = &m_dummy;
    m_dummy.prev = &m_dummy;
    if (other.m_size == 0) return;

    list_node_base* current = &m_dummy;
    list_node_base* to_copy = other.m_dummy.next;

    auto guard = jman::scope_guard([&] {
        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            m_clean_nodes_up_to_backwards(current);

            m_dummy.next = &m_dummy;
            m_dummy.prev = &m_dummy;
        }
    });

    for (size_type i = 0; i < other.m_size; ++i)
    {
        list_node* node = m_alloc_node();
        list_node_base* base_ptr = static_cast<list_node_base*>(node);

        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            // wrap it around a try-catch block, ive heard it has zero-cost overhead entering a try-catch block anywhere and the only overhead is the stack unwinding
            // so its cheap
            JMAN_TRY {
                jman::construct_at<T>(m_val_ptr(node), std::as_const(m_value(to_copy)));
            } JMAN_CATCH (...) {
                m_dealloc_node(node);
                JMAN_RETHROW;
            }
        } else {
            // safe to construct since its marked as nothrow
            jman::construct_at<T>(m_val_ptr(node), std::as_const(m_value(to_copy)));
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
linked_list<T>& linked_list<T>::operator=(const linked_list& other)
{
    if (this != &other)
    {
        const list_node_base* to_copy = other.m_dummy.next;
        list_node_base* current = &m_dummy;
        list_node_base* to_delete = m_dummy.next;
        size_type constructed = 0;

        auto guard = jman::scope_guard([&] {
            if constexpr (
                !std::is_nothrow_copy_constructible_v<T> 
                && !std::is_nothrow_copy_assignable_v<T> 
            ) {
                if (constructed > m_size) { // the failed node was one of the nodes that wasnt reused, so its safe to delete
                    list_node_base* prev = current->prev;
                    m_dealloc_node(current);

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

                m_value(current) = m_value(to_copy);

                to_delete = current->next;
                to_copy = to_copy->next;
                
                ++constructed;
            } else {
                // delete node and truncate
                list_node_base* next = to_delete->next;
                
                jman::destruct_at<T>(m_val_ptr(to_delete));
                m_dealloc_node(to_delete);

                to_delete = next;
            }
        }

        size_type diff = other.m_size - m_size;
        if (diff > other.m_size) diff = 0; // overflow

        for (size_type i = 0; i < diff; ++i)
        {
            // create new node
            list_node* node = m_alloc_node();
            list_node_base* base_ptr = static_cast<list_node_base*>(node);

            if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
                // wrap it around a try-catch block, ive heard it has zero-cost overhead entering a try-catch block anywhere and the only overhead is the stack unwinding
                // so its cheap
                JMAN_TRY {
                    jman::construct_at<T>(m_val_ptr(node), m_value(to_copy));
                } JMAN_CATCH (...) {
                    m_dealloc_node(node);
                    JMAN_RETHROW;
                }
            } else {
                // safe to construct since its marked as nothrow
                jman::construct_at<T>(m_val_ptr(node), m_value(to_copy));
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
linked_list<T>::linked_list(linked_list&& other) noexcept
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
linked_list<T>& linked_list<T>::operator=(linked_list&& other) noexcept
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
linked_list<T>::~linked_list() noexcept
{
    clear();
}

// ####################################################################################################################
// ------------------------------------------------ ACCESSING ELEMENTS ----------------------------------------------//
// ####################################################################################################################

template <typename T>
linked_list<T>::reference linked_list<T>::at(size_type index)
{   
    if (index >= m_size) JMAN_THROW(std::out_of_range("jman::linked_list<T>::at() failed: index provided out of range."));

    size_type half = (m_size - 1) >> 1;
    if (index < half) // start from the head
    {
        list_node_base* current = m_dummy.next;

        while (index > 0) {
            current = current->next;
            --index;
        }

        return m_value(current);
    } else { // start from the tail
        list_node_base* current = m_dummy.prev;

        while (index < (m_size - 1)) {
            current = current->prev;
            ++index;
        }

        return m_value(current);
    }
}
template <typename T>
linked_list<T>::const_reference linked_list<T>::at(size_type index) const
{
    if (index >= m_size) JMAN_THROW(std::out_of_range("jman::linked_list<T>::at() failed: index provided out of range."));

    size_type half = (m_size - 1) >> 1;
    if (index < half) // start from the head
    {
        list_node_base* current = m_dummy.next;

        while (index > 0) {
            current = current->next;
            --index;
        }

        return m_value(current);
    } else { // start from the tail
        list_node_base* current = m_dummy.prev;

        while (index < (m_size - 1)) {
            current = current->prev;
            ++index;
        }

        return m_value(current);
    }
}

template <typename T>
linked_list<T>::iterator linked_list<T>::find(const_reference value) noexcept
{
    if (empty()) return end();

    list_node_base* current = m_dummy.next;
    size_type counter = 0;

    while (current != &m_dummy) {
        if (m_value(current) == value) return iterator{current};
        current = current->next;
    }

    return end();
}
template <typename T>
linked_list<T>::const_iterator linked_list<T>::find(const_reference value) const noexcept
{
    if (empty()) return end();

    list_node_base* current = m_dummy.next;

    while (current != &m_dummy) {
        if (m_value(current) == value) return iterator{current};
        current = current->next;
    }

    return end();
}

// ####################################################################################################################
// -------------------------------------------------- RANGE MODIFIERS -----------------------------------------------//
// ####################################################################################################################

template <typename T>
void linked_list<T>::clear() noexcept
{
    m_clean_nodes_up_to_forward(m_dummy.next);

    m_dummy.next = m_dummy.prev = &m_dummy;
    m_size = 0;
}
template <typename T>
void linked_list<T>::resize(size_type count)
{
    if (count > m_size) {
        // increase
        list_node_base* current = &m_dummy;
        list_node_base* head = nullptr;

        auto guard = jman::scope_guard([&] {
            if constexpr (!std::is_nothrow_default_constructible_v<T>)
                m_clean_nodes_up_to_backwards(current);
        });

        for (size_type i = m_size; i < count; ++i)
        {
            list_node* node = m_alloc_node();
            list_node_base* base_ptr = static_cast<list_node_base*>(node);

            if constexpr (!std::is_nothrow_default_constructible_v<T>) {
                JMAN_TRY {
                    jman::construct_at<T>(m_val_ptr(node));
                } JMAN_CATCH (...) {
                    m_dealloc_node(node);
                    JMAN_RETHROW;
                }
            } else {
                // safe to construct since its marked as nothrow
                jman::construct_at<T>(m_val_ptr(node));
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
        list_node_base* current = m_dummy.prev;
        for (size_type i = m_size; i > count; --i)
        {
            list_node_base* prev = current->prev;
            
            jman::destruct_at<T>(m_val_ptr(current));
            m_dealloc_node(current);

            current = prev;
        }

        current->next = &m_dummy;
        m_dummy.prev = current;

        m_size = count;
    }
}
template <typename T>
void linked_list<T>::resize(size_type count, const_reference value)
{
    if (count > m_size) {
        // increase
        list_node_base* current = &m_dummy;
        list_node_base* head = nullptr;

        auto guard = jman::scope_guard([&] {
            if constexpr (!std::is_nothrow_copy_constructible_v<T>)
                m_clean_nodes_up_to_backwards(current);
        });

        for (size_type i = m_size; i < count; ++i)
        {
            list_node* node = m_alloc_node();
            list_node_base* base_ptr = static_cast<list_node_base*>(node);

            if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
                JMAN_TRY {
                    jman::construct_at<T>(m_val_ptr(node), value);
                } JMAN_CATCH (...) {
                    m_dealloc_node(node);
                    JMAN_RETHROW;
                }
            } else {
                // safe to construct since its marked as nothrow
                jman::construct_at<T>(m_val_ptr(node), value);
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
        list_node_base* current = m_dummy.prev;
        for (size_type i = m_size; i > count; --i)
        {
            list_node_base* prev = current->prev;
            
            jman::destruct_at<T>(m_val_ptr(current));
            m_dealloc_node(current);

            current = prev;
        }

        current->next = &m_dummy;
        m_dummy.prev = current;

        m_size = count;
    }
}

template <typename T>
void linked_list<T>::assign(size_type count, const_reference value)
{
    if (count == 0) return;
    list_node_base* current = &m_dummy;
    list_node_base* to_delete = m_dummy.next;
    size_type constructed = 0;

    auto guard = jman::scope_guard([&] {
        if constexpr (
            std::is_nothrow_copy_constructible_v<T> 
            && std::is_nothrow_copy_assignable_v<T> 
        ) {
            if (constructed > m_size) { // the failed node was one of the nodes that wasnt reused, so its safe to delete
                list_node_base* prev = current->prev;
                m_dealloc_node(current);

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

            m_value(current) = value;

            to_delete = current->next;
            
            ++constructed;
        } else {
            // delete node and truncate
            list_node_base* next = to_delete->next;
            
            jman::destruct_at<T>(m_val_ptr(to_delete));
            m_dealloc_node(to_delete);

            to_delete = next;
        }
    }

    size_type diff = count - m_size;
    if (diff > count) diff = 0; // overflow

    for (size_type i = 0; i < diff; ++i)
    {
        // create new node
        list_node* node = m_alloc_node();
        list_node_base* base_ptr = static_cast<list_node_base*>(node);

        if constexpr (!std::is_nothrow_copy_constructible_v<T>) {
            // wrap it around a try-catch block, ive heard it has zero-cost overhead entering a try-catch block anywhere and the only overhead is the stack unwinding
            // so its cheap
            JMAN_TRY {
                jman::construct_at<T>(m_val_ptr(node), value);
            } JMAN_CATCH (...) {
                m_dealloc_node(node);
                JMAN_RETHROW;
            }
        } else {
            // safe to construct since its marked as nothrow
            jman::construct_at<T>(m_val_ptr(node), value);
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
void linked_list<T>::sort()
{
    sort(std::less{});
}
template <typename T>
template <typename Compare> 
void linked_list<T>::sort(Compare comp)
{
    if (m_size < 2) return;

    // make the head and tail nodes point to nullptr to make the sorting process easier
    list_node_base* flatHead = m_dummy.next;
    m_dummy.prev->next = nullptr;
    flatHead->prev = nullptr; 

    for (size_type step = 1; step < m_size; step <<= 1) {
        list_node_base* curr = flatHead;
        list_node_base* prevTail = nullptr;
        flatHead = nullptr;

        while (curr) {
            list_node_base* left = curr;
            list_node_base* leftTail = nullptr;
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

            list_node_base* right = curr;
            list_node_base* rightTail = nullptr;
            size_type rightSize = 0;
            
            // grab the tail for the right node
            while (curr && rightSize < step) {
                rightTail = curr;
                curr = curr->next;
                rightSize++;
            }

            // disconnects the right with the rest of the list 
            if (rightTail) rightTail->next = nullptr;

            list_node_base* mergedHead = m_sort_merge(left, right, comp);

            // links the recent merged with the already sorted section
            if (prevTail) {
                prevTail->next = mergedHead;
                mergedHead->prev = prevTail;
            } else {
                flatHead = mergedHead;
            }

            list_node_base* mergedTail = mergedHead;
            while (mergedTail->next)
                mergedTail = mergedTail->next;
            prevTail = mergedTail;
        }
    }

    // reconnect the sorted nodes back to m_dummy
    m_dummy.next = flatHead;
    flatHead->prev = &m_dummy;

    // walk to the last real element
    list_node_base* finalTail = flatHead;
    while (finalTail->next) 
        finalTail = finalTail->next;
    finalTail->next = &m_dummy;
    m_dummy.prev = finalTail;
}
template <typename T>
linked_list<T>::size_type linked_list<T>::unique()
{
    if (m_size < 2) return 0;
    list_node_base* current = m_dummy.next;
    size_type removed = 0;

    while (current->next != &m_dummy) {
        list_node_base* next = current->next;

        if (m_value(current) == m_value(next))
        {
            jman::destruct_at(m_val_ptr(next));

            next->next->prev = current;
            current->next = next->next;

            m_dealloc_node(next);
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
linked_list<T>::size_type linked_list<T>::unique(BinaryPredicate predicate)
{
    if (m_size < 2) return 0;
    list_node_base* current = m_dummy.next;
    size_type removed = 0;

    while (current->next != &m_dummy) {
        list_node_base* next = current->next;

        if (predicate(current, next))
        {
            jman::destruct_at(m_val_ptr(next));

            next->next->prev = current;
            current->next = next->next;

            m_dealloc_node(next);
            --m_size;

            next = current;
            ++removed;
        }

        current = next;
    }

    return removed;
}
template <typename T>
void linked_list<T>::splice(const_iterator position, const linked_list& x)
{
    if (x.m_size == 0 || &x == this) return;

    auto insertAt = const_cast<list_node_base*>(position.m_val_ptr);

    list_node_base* current = &m_dummy; // for safety
    list_node_base* head = nullptr;
    list_node_base* to_copy = x.m_dummy.next;

    auto guard = jman::scope_guard([&] {
        if constexpr (!std::is_nothrow_default_constructible_v<T>)
            m_clean_nodes_up_to_backwards(current);
    });

    for (size_type i = 0; i < x.m_size; ++i)
    {
        list_node* node = m_alloc_node();
        list_node_base* base_ptr = static_cast<list_node_base*>(node);

        if constexpr (!std::is_nothrow_default_constructible_v<T>) {
            JMAN_TRY {
                jman::construct_at<T>(m_val_ptr(node), m_value(to_copy));
            } JMAN_CATCH (...) {
                m_dealloc_node(node);
                JMAN_RETHROW;
            }
        } else {
            // safe to construct since its marked as nothrow
            jman::construct_at<T>(m_val_ptr(node), m_value(to_copy));
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
void linked_list<T>::splice(const_iterator position, linked_list&& x) noexcept
{
    if (x.m_size == 0 || &x == this) return;

    auto insertAt = const_cast<list_node_base*>(position.m_val_ptr);
    insertAt->prev->next = x.m_dummy.next;

    x.m_dummy.next->prev = insertAt->prev;
    x.m_dummy.prev->next = insertAt;

    insertAt->prev = x.m_dummy.prev;

    m_size += x.m_size;
    
    x.m_dummy.next = x.m_dummy.prev = &x.m_dummy;
    x.m_size = 0;
}


template <typename T>
void linked_list<T>::reverse() noexcept
{
    if (m_size < 2) return;

    list_node_base* l = m_dummy.next;
    list_node_base* r = m_dummy.prev;

    size_type li = 0;
    size_type ri = m_size - 1;

    while (li < ri) {
        list_node_base* nextR = r->prev;
        list_node_base* nextL = l->next;

        m_swap_nodes(l, r);
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
void linked_list<T>::push_front(TT&& value)
{
    list_node* node = m_alloc_node();
    list_node_base* base_ptr = static_cast<list_node_base*>(node);
    
    jman::construct_at<T>(m_val_ptr(node), static_cast<T>(std::forward<TT>(value)));

    base_ptr->next = m_dummy.next;
    base_ptr->prev = &m_dummy;

    m_dummy.next->prev = base_ptr;
    m_dummy.next = base_ptr;

    ++m_size;
}


template <typename T>
template <typename TT>
requires std::is_convertible_v<TT, T>
void linked_list<T>::push_back(TT&& value)
{
    list_node* node = m_alloc_node();
    list_node_base* base_ptr = static_cast<list_node_base*>(node);
    
    jman::construct_at<T>(m_val_ptr(node), static_cast<T>(std::forward<TT>(value)));

    base_ptr->next = &m_dummy;
    base_ptr->prev = m_dummy.prev;

    m_dummy.prev->next = base_ptr;
    m_dummy.prev = base_ptr;

    ++m_size;
}

template <typename T>
template <typename ...Args>
requires std::is_constructible_v<T, Args...>
void linked_list<T>::emplace_front(Args&&... args)
{
    list_node* node = m_alloc_node();
    list_node_base* base_ptr = static_cast<list_node_base*>(node);
    
    jman::construct_at<T>(m_val_ptr(node), std::forward<Args>(args)...);

    base_ptr->next = m_dummy.next;
    base_ptr->prev = &m_dummy;

    m_dummy.next->prev = base_ptr;
    m_dummy.next = base_ptr;

    ++m_size;
}
template <typename T>
template <typename ...Args>
requires std::is_constructible_v<T, Args...>
void linked_list<T>::emplace_back(Args&&... args)
{
    list_node* node = m_alloc_node();
    list_node_base* base_ptr = static_cast<list_node_base*>(node);
    
    jman::construct_at<T>(m_val_ptr(node), std::forward<Args>(args)...);

    base_ptr->next = &m_dummy;
    base_ptr->prev = m_dummy.prev;

    m_dummy.prev->next = base_ptr;
    m_dummy.prev = base_ptr;

    ++m_size;
}

template <typename T>
void linked_list<T>::pop_front()
{
    if (empty()) return;

    list_node_base* node = m_dummy.next;
    jman::destruct_at(m_val_ptr(node));

    node->next->prev = node->prev;
    node->prev->next = node->next;

    m_dealloc_node(node);
    --m_size;
}
template <typename T>
void linked_list<T>::pop_back()
{
    if (empty()) return;

    list_node_base* node = m_dummy.next;
    jman::destruct_at(m_val_ptr(node));

    node->next->prev = node->prev;
    node->prev->next = node->next;

    m_dealloc_node(node);
    --m_size;
}

template <typename T>
template <typename TT>
requires std::is_convertible_v<TT, T>
linked_list<T>::iterator linked_list<T>::insert(size_type index, TT&& value)
{
    return insert(at(index), std::forward<TT>(value));
}
template <typename T>
template <typename ...Args>
requires std::is_constructible_v<T, Args...>
linked_list<T>::iterator linked_list<T>::emplace(size_type index, Args&&... args)
{
    return emplace(at(index), std::forward<Args>(args)...);
}

template <typename T>
template <typename TT>
requires std::is_convertible_v<TT, T>
linked_list<T>::iterator linked_list<T>::insert(iterator it, TT&& value)
{
    if (it == end()) return end();
    list_node* node = m_alloc_node();
    list_node_base* base_ptr = static_cast<list_node_base*>(node);
    
    jman::construct_at<T>(m_val_ptr(node), static_cast<T>(std::forward<TT>(value)));   
    
    base_ptr->next = it.m_val_ptr;
    base_ptr->prev = it.m_val_ptr->prev;

    it.m_val_ptr->prev->next = base_ptr;
    it.m_val_ptr->prev = base_ptr;

    ++m_size;

    return iterator{base_ptr};
}
template <typename T>
template <typename ...Args>
requires std::is_constructible_v<T, Args...>
linked_list<T>::iterator linked_list<T>::emplace(iterator it, Args&&... args)
{
    if (it == end()) return end();
    list_node* node = m_alloc_node();
    list_node_base* base_ptr = static_cast<list_node_base*>(node);
    
    jman::construct_at<T>(m_val_ptr(node), std::forward<Args>(args)...);   
    
    base_ptr->next = it.m_val_ptr;
    base_ptr->prev = it.m_val_ptr->prev;

    it.m_val_ptr->prev->next = base_ptr;
    it.m_val_ptr->prev = base_ptr;

    ++m_size;

    return iterator{base_ptr};
}

template <typename T>
void linked_list<T>::erase(iterator it)
{
    if (empty()) return;
    if (it == end()) return;
    list_node_base* node = it.m_val_ptr;
    jman::destruct_at(m_val_ptr(node));

    node->next->prev = node->prev;
    node->prev->next = node->next;

    m_dealloc_node(node);
    --m_size;
}
template <typename T>
void linked_list<T>::erase(size_type index)
{
    erase(at(index));
}

} // namespace jman