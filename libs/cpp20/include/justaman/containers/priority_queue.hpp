#pragma once

#include <stdexcept>
#include <type_traits>
#include <utility>
#include <justaman/containers/vector.hpp>
#include <justaman/utils/attributes.h>

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
        struct node 
        {
            std::size_t priority;
            T value;
        };
        
        jman::vector<node> m_data {};

        private:
            void __swim(std::size_t index)
                noexcept(std::is_nothrow_swappable_v<T>); // fix up
            void __sink(std::size_t index)
                noexcept(std::is_nothrow_swappable_v<T>); // fix down
        
        public: 
            priority_queue() = default;

            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool empty() const noexcept { return m_data.empty(); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t size() const noexcept { return m_data.size(); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t capacity() const noexcept { return m_data.capacity(); }
            /** @note that changing anything in the front and not popping it after it might cause "invalid" results, o
                only use this func when u need to move a big object outside */
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T& peek() noexcept { return m_data.front().value; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const T& peek() const noexcept { return m_data.front().value; }

            JMAN_FORCE_INLINE void clear() { m_data.clear(); }
            JMAN_FORCE_INLINE void shrink_to_fit() { m_data.shrink_to_fit(); }
            JMAN_FORCE_INLINE void reserve(std::size_t count) { m_data.reserve(count); }

            template <typename TT>
            requires std::is_convertible_v<TT, T>
            void push(TT&& value, std::size_t priority);
            template <typename ...Args>
            void emplace(std::size_t priority, Args&& ...args);
            void pop();
    };

}

template <typename T>
void jman::priority_queue<T>::__swim(std::size_t index)
    noexcept(std::is_nothrow_swappable_v<T>)
{
    while (index > 0 && m_data[(index - 1) / 2].priority < m_data[index].priority)
    {   // continue swaping until the index becomes the root or when meeting someone with more priority
        std::size_t parent = (index - 1) / 2;
        std::swap(m_data[(index - 1) / 2], m_data[index]);
        index = parent;
    }
}
template <typename T>
void jman::priority_queue<T>::__sink(std::size_t index)
    noexcept(std::is_nothrow_swappable_v<T>)
{
    while (2 * index + 1 < m_data.size()) // if atleast left child exists
    {
        std::size_t j = 2 * index + 1;

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


template <typename T>
template <typename TT>
requires std::is_convertible_v<TT, T>
void jman::priority_queue<T>::push(TT&& value, std::size_t priority)
{
    m_data.emplace_back(priority, static_cast<T>(std::forward<TT>(value)));
    __swim(m_data.size() - 1);
}

template <typename T>
template <typename ...Args>
void jman::priority_queue<T>::emplace(std::size_t priority, Args&& ...args)
{
    m_data.emplace_back(priority, std::forward<Args>(args)...);
    __swim(m_data.size() - 1);
}

template <typename T>
void jman::priority_queue<T>::pop()
{
    if JMAN_UNLIKELY(m_data.empty()) throw std::out_of_range("jman::priority_queue<T>::pop failed: queue is empty.");

    // node value = std::move_if_noexcept(m_data.front());

    m_data.front() = std::move_if_noexcept(m_data.back());
    m_data.pop_back();

    __sink(0);
}