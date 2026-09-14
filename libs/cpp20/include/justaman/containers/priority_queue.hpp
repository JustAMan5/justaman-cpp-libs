#pragma once

#include <justaman/containers/vector.hpp>
#include <type_traits>
#include <utility>

/**
    priority queue structure:

        left child of index i = 2 * i + 1
        right child of index i = 2 * i + 2

        parent of index i = (i - 1) // 2

        these rules r for 0 indexed arrays (index starts at 0)
*/

namespace jman
{

    /**
        @brief
            a max heap priority queue wrapped around a vector, this implementation uses a raw priority number value
            instead of a comparator function to determine which has more priority
    */
    template <typename T>
    class priority_queue 
    {
        public:
            using value_type        = T;
            using size_type         = std::size_t;
            using reference         = T&;
            using const_reference   = const T&;
            using pointer           = T*;
            using const_pointer     = const T*;
        
            priority_queue() = default;

            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool              empty() const noexcept      { return m_data.empty(); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type         size() const noexcept       { return m_data.size(); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type         capacity() const noexcept   { return m_data.capacity(); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference   peek() const noexcept       { return m_data.front().value; }
            /** @warning that this method only exists to move the current element and directly pop it after */
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference         peek() noexcept             { return m_data.front().value; }

            JMAN_FORCE_INLINE void                              clear()                     { m_data.clear(); }
            JMAN_FORCE_INLINE void                              shrink_to_fit()             { m_data.shrink_to_fit(); }
            JMAN_FORCE_INLINE void                              reserve(size_type count)    { m_data.reserve(count); }

            template <typename TT> requires std::is_convertible_v<TT, T>
            void                                                push(size_type priority, TT&& value);
            template <typename ...Args> requires std::is_constructible_v<T, Args...>
            void                                                emplace(size_type priority, Args&& ...args);
            void                                                pop();

        private:
            struct node 
            {
                size_type   priority;
                value_type  value;
            };
            
            jman::vector<node> m_data {};

        private:
            void m_swim(size_type index)
                noexcept(std::is_nothrow_swappable_v<T>); // fix up
            void m_sink(size_type index)
                noexcept(std::is_nothrow_swappable_v<T>); // fix down
    };

    /** not implemented yet, only declared */
    template <typename T, typename Comparator = std::less<T>>
    class priority_queue_comp;
} // namespace jman

namespace jman 
{

// #####################################################################################
// -------------------------------- PRIVATE METHODS ----------------------------------//
// #####################################################################################

template <typename T>
void priority_queue<T>::m_swim(size_type index)
    noexcept(std::is_nothrow_swappable_v<T>)
{
    while (index > 0 && m_data[(index - 1) / 2].priority < m_data[index].priority)
    {   // continue swaping until the index becomes the root or when meeting someone with more priority
        size_type parent = (index - 1) / 2;
        std::swap(m_data[(index - 1) / 2], m_data[index]);
        index = parent;
    }
}
template <typename T>
void priority_queue<T>::m_sink(size_type index)
    noexcept(std::is_nothrow_swappable_v<T>)
{
    while (2 * index + 1 < m_data.size()) // if atleast left child exists
    {
        size_type j = 2 * index + 1;

        // if right exists and larger than left, make it the target
        if (m_data[j].priority < m_data[j + 1].priority && j != m_data.size() - 1)
            ++j;

        // if parent is already larger than the largest child, we are done
        if (m_data[index].priority >= m_data[j].priority)
            break;

        std::swap(m_data[index], m_data[j]);
        index = j;
    }
}

// #####################################################################################
// -------------------------------- PUBLIC METHODS ----------------------------------//
// #####################################################################################


template <typename T>
template <typename TT> requires std::is_convertible_v<TT, T>
void priority_queue<T>::push(size_type priority, TT&& value)
{
    m_data.emplace_back(priority, static_cast<T>(std::forward<TT>(value)));
    m_swim(m_data.size() - 1);
}

template <typename T>
template <typename ...Args> requires std::is_constructible_v<T, Args...>
void priority_queue<T>::emplace(size_type priority, Args&& ...args)
{
    m_data.emplace_back(priority, std::forward<Args>(args)...);
    m_swim(m_data.size() - 1);
}

template <typename T>
void priority_queue<T>::pop()
{
    if (m_data.empty()) return;

    // node value = std::move_if_noexcept(m_data.front());

    m_data.front() = std::move_if_noexcept(m_data.back());
    m_data.pop_back();

    m_sink(0);
}


} // namespace jman