#pragma once

#include <functional>
#include <justaman/containers/vector.hpp>
#include <concepts>
#include <initializer_list>
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
    struct use_external_priority_t {};

    /**
        @brief
            a max heap priority queue wrapped around a vector, the comparator function must return true if 
            @a has LOWER priority than @b, false if otherwise.
            \n
            if you want instead to use a comp func, use an external priority value, u can pass on the 
            PolicyOrComp template a jman::use_external_priority_t
    */
    template <typename T, typename PolicyOrComp = std::less<T>>
    class priority_queue 
    {
        static_assert(
            std::same_as<PolicyOrComp, use_external_priority_t> || std::predicate<PolicyOrComp, T, T>, 
            "Must be a valid comparator func or be use_external_priority_t{}"
        );
        static constexpr bool is_keyed = std::is_same_v<PolicyOrComp, use_external_priority_t>;

        public:
            using value_type        = T;
            using size_type         = std::size_t;
            using reference         = T&;
            using const_reference   = const T&;
            using pointer           = T*;
            using const_pointer     = const T*;
        
            priority_queue() noexcept = default;

            priority_queue(std::initializer_list<std::pair<T, std::size_t>> list) requires(is_keyed);
            priority_queue(std::initializer_list<T> list) requires(!is_keyed);

            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool              empty() const noexcept      { return m_data.empty(); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type         size() const noexcept       { return m_data.size(); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE size_type         capacity() const noexcept   { return m_data.capacity(); }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE const_reference   peek() const noexcept       { return m_data.front().value; }
            /** @warning that this method only exists to move the current element and directly pop it after */
            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference         peek() noexcept             { return m_data.front().value; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE reference         top_priority() noexcept requires(is_keyed) { return m_data.front().priority; }

            JMAN_FORCE_INLINE void                              clear()                     { m_data.clear(); }
            JMAN_FORCE_INLINE void                              shrink_to_fit()             { m_data.shrink_to_fit(); }
            JMAN_FORCE_INLINE void                              reserve(size_type count)    { m_data.reserve(count); }

            template <typename TT>      requires std::is_convertible_v<TT, T>
            void                                                push(size_type priority, TT&& value) requires(is_keyed);
            template <typename ...Args> requires std::is_constructible_v<T, Args...>
            void                                                emplace(size_type priority, Args&& ...args) requires(is_keyed);

            template <typename TT>      requires std::is_convertible_v<TT, T>
            void                                                push(TT&& value) requires(!is_keyed);
            template <typename ...Args> requires std::is_constructible_v<T, Args...>
            void                                                emplace(Args&& ...args) requires(!is_keyed);

            void                                                pop();

        private:
            struct empty_comp {};

            struct keyed_node 
            {
                size_type   priority;
                value_type  value;
            };

            struct value_node  
            {
                value_type  value;
            };

            using note_t = std::conditional_t<is_keyed, keyed_node, value_node>;

            jman::vector<note_t, size_type> m_data {};
            JMAN_NO_UNIQUE_ADDRESS std::conditional_t<is_keyed, empty_comp, PolicyOrComp> m_comp;
        private:
            void m_swim(size_type index)
                noexcept(std::is_nothrow_swappable_v<T>) requires(is_keyed); // fix up
            void m_sink(size_type index)
                noexcept(std::is_nothrow_swappable_v<T>) requires(is_keyed); // fix down

            void m_swim(size_type index) requires(!is_keyed); // fix up
            void m_sink(size_type index) requires(!is_keyed); // fix down
    };
} // namespace jman

namespace jman 
{

// #####################################################################################
// -------------------------------- PRIVATE METHODS ----------------------------------//
// #####################################################################################

template <typename T, typename PolicyOrComp>
void priority_queue<T, PolicyOrComp>::m_swim(size_type index)
    noexcept(std::is_nothrow_swappable_v<T>) requires(is_keyed)
{
    while (index > 0 && m_data[(index - 1) / 2].priority < m_data[index].priority)
    {   // continue swaping until the index becomes the root or when meeting someone with more priority
        size_type parent = (index - 1) / 2;
        std::swap(m_data[(index - 1) / 2], m_data[index]);
        index = parent;
    }
}
template <typename T, typename PolicyOrComp>
void priority_queue<T, PolicyOrComp>::m_sink(size_type index)
    noexcept(std::is_nothrow_swappable_v<T>) requires(is_keyed)
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

template <typename T, typename PolicyOrComp>
void priority_queue<T, PolicyOrComp>::m_swim(size_type index) requires(!is_keyed)
{
    while (index > 0 && m_comp(m_data[(index - 1) / 2].value, m_data[index].value))
    {   // continue swaping until the index becomes the root or when meeting someone with more priority
        size_type parent = (index - 1) / 2;
        std::swap(m_data[(index - 1) / 2], m_data[index]);
        index = parent;
    }
}
template <typename T, typename PolicyOrComp>
void priority_queue<T, PolicyOrComp>::m_sink(size_type index) requires(!is_keyed)
{
    while (2 * index + 1 < m_data.size()) // if atleast left child exists
    {
        size_type j = 2 * index + 1;

        // if right exists and larger than left, make it the target
        if (m_comp(m_data[j].value, m_data[j + 1].value) && j != m_data.size() - 1)
            ++j;

        // if parent is already larger than the largest child, we are done
        if (m_comp(m_data[j].value, m_data[index].value))
            break;

        std::swap(m_data[index], m_data[j]);
        index = j;
    }
}


// #####################################################################################
// ---------------------------------- CONSTRUCTORS ----------------------------------//
// #####################################################################################

template <typename T, typename PolicyOrComp>
priority_queue<T, PolicyOrComp>::priority_queue(std::initializer_list<std::pair<T, std::size_t>> list) requires(is_keyed)
{
    size_type length = list.size();
    m_data.reserve(length);

    for (const auto& val: list)
        m_data.emplace_back(val.second, val.first);

    if (length < 2) return;

    // builds heap in place
    for (size_type i = length / 2; i-- > 0;)
        while (true)
        {
            size_type largest = i;
            size_type l = 2 * i + 1;
            size_type r = 2 * i + 2;

            // if left child is larger set it as largest
            if (l < length && m_data[l].priority > m_data[largest].priority)
                largest = l;

            // if right child is larger set it as largest
            if (r < length && m_data[r].priority > m_data[largest].priority)
                largest = r;

            // if largest is not the root swap them and heapify the sub tree
            if (largest == i)
                break;

            std::swap(m_data[i], m_data[largest]);
            i = largest;
        }
}

template <typename T, typename PolicyOrComp>
priority_queue<T, PolicyOrComp>::priority_queue(std::initializer_list<T> list) requires(!is_keyed)
{
    size_type length = list.size();
    m_data.reserve(length);

    for (const auto& val: list)
        m_data.emplace_back(val);

    if (length < 2) return;

    // builds heap in place
    for (size_type i = length / 2; i-- > 0;)
        while (true)
        {
            size_type largest = i;
            size_type l = 2 * i + 1;
            size_type r = 2 * i + 2;

            // if left child is larger set it as largest
            if (l < length && !m_comp(m_data[l].value, m_data[largest].value))
                largest = l;

            // if right child is larger set it as largest
            if (r < length && !m_comp(m_data[r].value, m_data[largest].value))
                largest = r;

            // if largest is not the root swap them and heapify the sub tree
            if (largest == i)
                break;

            std::swap(m_data[i], m_data[largest]);
            i = largest;
        }
}

// #####################################################################################
// -------------------------------- PUBLIC METHODS ----------------------------------//
// #####################################################################################

template <typename T, typename PolicyOrComp>
template <typename TT> requires std::is_convertible_v<TT, T>
void priority_queue<T, PolicyOrComp>::push(size_type priority, TT&& value) requires(is_keyed)
{
    m_data.emplace_back(priority, static_cast<T>(std::forward<TT>(value)));
    m_swim(m_data.size() - 1);
}

template <typename T, typename PolicyOrComp>
template <typename ...Args> requires std::is_constructible_v<T, Args...>
void priority_queue<T, PolicyOrComp>::emplace(size_type priority, Args&& ...args) requires(is_keyed)
{
    m_data.emplace_back(priority, std::forward<Args>(args)...);
    m_swim(m_data.size() - 1);
}

template <typename T, typename PolicyOrComp>
template <typename TT> requires std::is_convertible_v<TT, T>
void priority_queue<T, PolicyOrComp>::push(TT&& value) requires(!is_keyed)
{
    m_data.emplace_back(static_cast<T>(std::forward<TT>(value)));
    m_swim(m_data.size() - 1);
}

template <typename T, typename PolicyOrComp>
template <typename ...Args> requires std::is_constructible_v<T, Args...>
void priority_queue<T, PolicyOrComp>::emplace(Args&& ...args) requires(!is_keyed)
{
    m_data.emplace_back(std::forward<Args>(args)...);
    m_swim(m_data.size() - 1);
}

template <typename T, typename PolicyOrComp>
void priority_queue<T, PolicyOrComp>::pop()
{
    if (m_data.empty()) return;

    // node value = std::move_if_noexcept(m_data.front());

    m_data.front() = std::move_if_noexcept(m_data.back());
    m_data.pop_back();

    m_sink(0);
}


} // namespace jman