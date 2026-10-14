#pragma once

#include "numeric.hpp"
#include <justaman/errors/safe_exceptions.hpp>
#include <justaman/memory/lifetime.hpp>
#include <justaman/utils/scope.hpp>
#include <justaman/utils/attributes.h>
#include <bit>
#include <iterator>
#include <type_traits>
#include <cstring>

namespace jman 
{
    template <typename Iterator, typename Comp>
    concept is_nothrow_sortable_comp =
        std::random_access_iterator<Iterator> &&
        std::is_nothrow_invocable_r_v<
            bool, 
            Comp, 
            std::iter_reference_t<Iterator>, 
            std::iter_reference_t<Iterator>
        > && 
        // ensure elements can be swapped safely
        std::is_nothrow_swappable_v<std::iter_value_t<Iterator>> &&
        // ensure elements can be moved/assigned safely during sorting partitioning
        std::is_nothrow_move_constructible_v<std::iter_value_t<Iterator>> &&
        std::is_nothrow_move_assignable_v<std::iter_value_t<Iterator>>;

    template <typename Iterator>
    concept is_nothrow_sortable = 
        is_nothrow_sortable_comp<Iterator, std::less<std::iter_value_t<Iterator>>>;


    /**
        @brief 
            qsort (aka. quick sort) swaps elements acording to a pivot element that tells if the others r smaller or larger
            best time:      O(nlog n)
            average time:   O(nlog n)
            worst time:     O(n^2)
            worst space:    O(log n)
            \n
            best general purpose sort due to its friendly cache locality
    */
    ///@{
    template <std::random_access_iterator Iterator, std::indirect_strict_weak_order<Iterator> Compare>
    constexpr void qsort(Iterator start, Iterator end, Compare comp)
        noexcept(is_nothrow_sortable_comp<Iterator, Compare>);

    template <std::random_access_iterator Iterator>
    JMAN_FORCE_INLINE constexpr void qsort(Iterator start, Iterator end)
        noexcept(is_nothrow_sortable<Iterator>);
    ///@}

    
    /**
        @brief
            msort (aka. merge sort) copies the array in sub arrays and sorts those sub arrays and merges them to make the final result
            best time:      O(nlog n)
            average time:   O(nlog n)
            worst time:     O(nlog n)
            worst space:    O(n)
            \n
            best for sorting large linkedlists or datasets
        @note 
            i think it can be used aswell for linked lists when implemented properly, but this version is just for cotnainers with random access iters.
            uses the same tricks in this post https://www.baeldung.com/cs/non-recursive-merge-sort
    */
    ///@{
    template <std::random_access_iterator Iterator, std::indirect_strict_weak_order<Iterator> Compare>
    constexpr void msort(Iterator start, Iterator end, Compare comp); // no noexcept since it needs to allocate space for the temp array for the copies

    template <std::random_access_iterator Iterator>
    JMAN_FORCE_INLINE constexpr void msort(Iterator start, Iterator end);
    ///@}
    /**
        @brief 
            hsort (aka. heap sort) treats the array as a heap / priority queue and keeps poping the front element and rearranging the array
            best time:      O(nlog n)
            average time:   O(nlog n)
            worst time:     O(nlog n)
            worst space:    O(1)
            \n
            best for embedded or real-time systems where memory space is tight
            dont use when raw speed is prioritized due to its poor cache friendliness
    */
    ///@{
    template <std::random_access_iterator Iterator, std::indirect_strict_weak_order<Iterator> Compare>
    constexpr void hsort(Iterator start, Iterator end, Compare comp)
        noexcept(is_nothrow_sortable_comp<Iterator, Compare>);

    template <std::random_access_iterator Iterator>
    JMAN_FORCE_INLINE constexpr void hsort(Iterator start, Iterator end)
        noexcept(is_nothrow_sortable<Iterator>);
    ///@}
    /**
        @brief 
            isort (aka. insertion sort) works by inserting elements into their correct position in a sorted portion of the list
            best time:      O(n)
            average time:   O(n^2)
            worst time:     O(n^2)
            worst space:    O(1)
            \n
            use it for small datasets (around 20 or less) or datasets that are nearly sorted
    */
    ///@{
    template <std::random_access_iterator Iterator, std::indirect_strict_weak_order<Iterator> Compare>
    constexpr void isort(Iterator start, Iterator end, Compare comp)
        noexcept(is_nothrow_sortable_comp<Iterator, Compare>);

    template <std::random_access_iterator Iterator>
    JMAN_FORCE_INLINE constexpr void isort(Iterator start, Iterator end)
        noexcept(is_nothrow_sortable<Iterator>);
    ///@}
    /**
        @brief 
            ssort (aka. selection sort) 
            iterates linearly trough the list and searches in the unsorted portion (after the current index) for the minimum or largest element and swaps them
            \n
            best time:      O(n^2)
            average time:   O(n^2)
            worst time:     O(n^2)
            worst space:    O(1)
            \n
            only use selection sort when writting to memory or swapping elements is very expensive
            for any other scenario it performs very poorly
    */
    ///@{
    template <std::random_access_iterator Iterator, std::indirect_strict_weak_order<Iterator> Compare>
    constexpr void ssort(Iterator start, Iterator end, Compare comp)
        noexcept(is_nothrow_sortable_comp<Iterator, Compare>);

    template <std::random_access_iterator Iterator>
    JMAN_FORCE_INLINE constexpr void ssort(Iterator start, Iterator end)
        noexcept(is_nothrow_sortable<Iterator>);
    ///@}
    /**
        @brief 
            sort (will reffer for intro sort)
            is an hybrid style that divides the array in portions like quick sort and performs insertion sort or heap sort depending on the sub array depth and size
            \n
            best time:      O(nlog n)
            average time:   O(nlog n)
            worst time:     O(nlog n)
            worst space:    O(log n)
            \n
            best for general purpose large randomly unordered arrays
    */
    ///@{
    template <std::random_access_iterator Iterator, std::indirect_strict_weak_order<Iterator> Compare>
    constexpr void sort(Iterator start, Iterator end, Compare comp)
        noexcept(is_nothrow_sortable_comp<Iterator, Compare>);

    template <std::random_access_iterator Iterator>
    constexpr void sort(Iterator start, Iterator end)
        noexcept(jman::is_nothrow_sortable<Iterator>);
    ///@}
} // namespace jman

// #######################################################################################
// --------------------------------- INTERNAL HELPERS ----------------------------------//
// #######################################################################################

namespace jman::detail
{

template <std::random_access_iterator Iterator>
struct intro_sort_task 
{
    using diff_t = typename std::iterator_traits<Iterator>::difference_type;

    diff_t left;
    diff_t right;
    int depth;
};

template <std::random_access_iterator Iterator, std::indirect_strict_weak_order<Iterator> Comp>
constexpr Iterator qsort_partition(Iterator start, Iterator end, Comp comp)
    noexcept(jman::is_nothrow_sortable_comp<Iterator, Comp>) 
{
    auto mid = start + (end - start) / 2;
    auto last = --end;

    if (comp(*mid, *start)) std::swap(*mid, *start);
    if (comp(*last, *start)) std::swap(*last, *start);
    if (comp(*mid, *last)) std::swap(*mid, *last);
    
    auto&& pivot = *last;
    auto i = start;

    for (auto j = start; j != last; ++j) {
        if (comp(*j, pivot)) {
            std::swap(*i, *j);
            ++i;
        }
    }
    
    std::swap(*i, *last);

    return i;
}

template <std::random_access_iterator Iterator, std::indirect_strict_weak_order<Iterator> Comp>
constexpr void hsort_heapify(Iterator start, Iterator end, typename std::iterator_traits<Iterator>::difference_type i, Comp comp) 
    noexcept(jman::is_nothrow_sortable_comp<Iterator, Comp>)
{
    using diff_t = typename std::iterator_traits<Iterator>::difference_type;
    diff_t length = end - start;
    
    while (true)
    {
        diff_t largest = i;
        diff_t l = 2 * i + 1;
        diff_t r = 2 * i + 2;

        // if left child is larger set it as largest
        if (l < length && !comp(start[l], start[largest]))
            largest = l;

        // if right child is larger set it as largest
        if (r < length && !comp(start[r], start[largest]))
            largest = r;

        // if largest is not the root swap them and heapify the sub tree
        if (largest == i)
            break;

        std::swap(start[i], start[largest]);
        i = largest;
    }
}

template <std::random_access_iterator Iterator, std::indirect_strict_weak_order<Iterator> Comp>
constexpr void msort_merge(
    Iterator L1, Iterator R1, 
    Iterator L2, Iterator R2, 
    std::iter_value_t<Iterator>* temp,
    Comp comp
)
    noexcept(jman::is_nothrow_sortable_comp<Iterator, Comp>)
{
    using diff_t = typename std::iterator_traits<Iterator>::difference_type;
    diff_t i = 0;

    while (L1 < R1 && L2 < R2)
    {
        if (comp(*L1, *L2)) {
            jman::construct_at(temp + i, std::move_if_noexcept(*L1));
            ++i;
            ++L1;
        }
        else {
            jman::construct_at(temp + i, std::move_if_noexcept(*L2));
            ++i;
            ++L2;
        }
    }

    // these two r in case there is any element left that we cant compare with another

    while (L1 < R1) 
    {
        jman::construct_at(temp + i, std::move_if_noexcept(*L1));
        ++i;
        ++L1;
    }

    while (L2 < R2)
    {
        
        jman::construct_at(temp + i, std::move_if_noexcept(*L2));
        ++i;
        ++L2;
    }
}

} // namespace jman::detail

// #######################################################################################
// ---------------------------------- IMPLEMENTATIONS ----------------------------------//
// #######################################################################################

namespace jman 
{

// #################################################################################
// --------------------------------- QUICK SORT ----------------------------------//
// #################################################################################

template <std::random_access_iterator Iterator, std::indirect_strict_weak_order<Iterator> Compare>
constexpr void qsort(Iterator start, Iterator end, Compare comp)
    noexcept(jman::is_nothrow_sortable_comp<Iterator, Compare>)
{
    if (end - start <= 1) return; // must have 2 or more elements

    /*
        this is for worst case scenario when the array contains 2^64 elements
        for every sub array it stores 2 iterators / pointers
        if we say that when dividing the array the largest will contain half of it: 2^63
        and the next division will have 2^62. if we continue this proccess we can end up with
        64 partitions, since we store the start and end we multiply by two so we end up with
        a 128 maximum that in reality is just 1kb of stack usage, very small for modern devices
    */
    Iterator stack[128];
    int top = -1;

    stack[++top] = start;
    stack[++top] = end;
    
    while (top >= 0)
    {
        end = stack[top--];
        start = stack[top--];

        // auto pivot = end - 1;
        // auto i = start;

        // for (auto j = start; j != end; ++j)
        // {
        //     if (comp(*j, *pivot)) {
        //         std::swap(*i, *j); 
        //         ++i;
        //     }
        // }
        // std::swap(*i, *pivot);

        auto newPivot = detail::qsort_partition(start, end, comp);

        if (newPivot > start) {
            stack[++top] = start;
            stack[++top] = newPivot;
        }
        if (newPivot + 1 < end) {
            stack[++top] = newPivot + 1;
            stack[++top] = end;
        }
    }
}

template <std::random_access_iterator Iterator>
JMAN_FORCE_INLINE constexpr void qsort(Iterator first, Iterator last)
    noexcept(is_nothrow_sortable<Iterator>)
{
    qsort(first, last, std::less{});
}

// #################################################################################
// --------------------------------- MERGE SORT ----------------------------------//
// #################################################################################


template <std::random_access_iterator Iterator, std::indirect_strict_weak_order<Iterator> Compare>
constexpr void msort(Iterator start, Iterator end, Compare comp)
{
    using diff_t = typename std::iterator_traits<Iterator>::difference_type;
    using T = std::iter_value_t<Iterator>;

    diff_t length = end - start;
    if (length <= 1) return;

    std::size_t result;
    if JMAN_UNLIKELY(!jman::validate_mul<std::size_t>(sizeof(T), static_cast<std::size_t>(length), &result))
        JMAN_THROW(std::overflow_error("jman::msort<Iterator, Compare> failed: calculated capacity for the merge sort temporary buffer overflows the std::size_t."));
    
    T* tmp = static_cast<T*>(::operator new(result, std::align_val_t{alignof(T)}));
    jman::scope_guard tmp_guard {[tmp](){::operator delete(static_cast<void*>(tmp), std::align_val_t{alignof(T)});}};

    std::size_t len = 1;
    std::size_t u_length = static_cast<std::size_t>(length);

    while (len < u_length)
    {
        std::size_t i = 0;
        while (i < u_length)
        {
            std::size_t L1 = i;
            std::size_t R1 = i + len;
            std::size_t L2 = i + len;
            std::size_t R2 = i + 2 * len;

            if (L2 >= u_length)
                break;

            if (R2 >= u_length)
                R2 = u_length;

            detail::msort_merge(
                start + static_cast<diff_t>(L1),
                start + static_cast<diff_t>(R1),
                start + static_cast<diff_t>(L2), 
                start + static_cast<diff_t>(R2),
                tmp,
                comp
            );

            std::size_t num_elements = R2 - L1;

            if constexpr (std::is_trivially_copyable_v<T>)
                std::memcpy(start + L1, tmp, sizeof(T) * num_elements);
            else 
                for (std::size_t j = 0; j < R2 - L1; ++j)
                    start[i + j] = std::move_if_noexcept(tmp[j]);

            jman::destruct_at(tmp, num_elements);
    
            i += len << 1; // same as len * 2, the compiler will still use the shift even when doing len * 2 but this looks more aura farming
        }
        len <<= 1; // copy the aura from above like a cool idiot that will get 100 different runtime errors for this ugly ass code
    }
}

template <std::random_access_iterator Iterator>
JMAN_FORCE_INLINE constexpr void msort(Iterator first, Iterator last)
{
    msort(first, last, std::less{});
}

// #################################################################################
// --------------------------------- HEAP SORT ----------------------------------//
// #################################################################################


template <std::random_access_iterator Iterator, std::indirect_strict_weak_order<Iterator> Compare>
constexpr void hsort(Iterator start, Iterator end, Compare comp)
    noexcept(is_nothrow_sortable_comp<Iterator, Compare>)
{
    using diff_t = typename std::iterator_traits<Iterator>::difference_type;

    diff_t length = end - start;
    if (length <= 1) return;

    // build heap inplace
    for (diff_t i = length / 2; i-- > 0;)
        detail::hsort_heapify(start, end, i, comp);

    // one by one extract top elements from the heap and swap them with the back
    for (diff_t i = length; i-- > 0;) {

        // Move current root to end
        std::swap(start[0], start[i]);

        // rearrange the heap again
        detail::hsort_heapify(start, start + i, 0, comp);
    }
}

template <std::random_access_iterator Iterator>
JMAN_FORCE_INLINE constexpr void hsort(Iterator first, Iterator last)
    noexcept(is_nothrow_sortable<Iterator>)
{
    hsort(first, last, std::less{});
}

// #################################################################################
// ------------------------------- INSERTION SORT --------------------------------//
// #################################################################################


template <std::random_access_iterator Iterator, std::indirect_strict_weak_order<Iterator> Compare>
constexpr void isort(Iterator start, Iterator end, Compare comp)
    noexcept(is_nothrow_sortable_comp<Iterator, Compare>)
{
    using diff_t = typename std::iterator_traits<Iterator>::difference_type;

    diff_t length = end - start;
    if (length <= 1) return;

    using T = std::iter_value_t<Iterator>;

    // start with the second element since the first one is assumed its already sorted
    for (diff_t i = 1; i < length; ++i) {
        T value = std::move_if_noexcept(start[i]);
        diff_t j = i - 1;

        /* Move elements of the array that are
        greater than value to one position ahead
        of their current position */
        while (j < length && !comp(start[j], value)) {
            start[j + 1] = std::move_if_noexcept(start[j]);
            --j;
        }
        start[j + 1] = std::move_if_noexcept(value);
    }
}

template <std::random_access_iterator Iterator>
JMAN_FORCE_INLINE constexpr void isort(Iterator first, Iterator last)
    noexcept(is_nothrow_sortable<Iterator>)
{
    isort(first, last, std::less{});
}


// #################################################################################
// ------------------------------- SELECTION SORT --------------------------------//
// #################################################################################


template <std::random_access_iterator Iterator, std::indirect_strict_weak_order<Iterator> Compare>
constexpr void ssort(Iterator start, Iterator end, Compare comp)
    noexcept(is_nothrow_sortable_comp<Iterator, Compare>)
{
    using diff_t = typename std::iterator_traits<Iterator>::difference_type;

    diff_t length = end - start;
    if (length <= 1) return;

    for (diff_t i = 0; i < length - 1; ++i) {

        diff_t min_idx = i;

        // iterate through the unsorted portion
        // to find the actual minimum
        for (diff_t j = i + 1; j < length; ++j)
            if (comp(start[j], start[min_idx])) {

                // update min_idx if a smaller element is found
                min_idx = j; 
            }

        // move minimum element to its correct position
        std::swap(start[i], start[min_idx]);
    }
}

template <std::random_access_iterator Iterator>
JMAN_FORCE_INLINE constexpr void ssort(Iterator first, Iterator last)
    noexcept(is_nothrow_sortable<Iterator>)
{
    ssort(first, last, std::less{});
}


// #################################################################################
// --------------------------------- INTRO SORT ----------------------------------//
// #################################################################################

template <std::random_access_iterator Iterator, std::indirect_strict_weak_order<Iterator> Compare>
constexpr void sort(Iterator start, Iterator end, Compare comp)
    noexcept(is_nothrow_sortable_comp<Iterator, Compare>)
{
    using diff_t = typename std::iterator_traits<Iterator>::difference_type;

    diff_t length = end - start;
    if (length <= 1) return;

    int depthLimit = 2 * std::bit_width(static_cast<std::make_unsigned_t<diff_t>>(length));

    detail::intro_sort_task<Iterator> stack[128];
    int top = -1;
    stack[++top] = {0, length - 1, depthLimit};


    while (top >= 0) 
    {
        detail::intro_sort_task<Iterator> task = stack[top--];
        diff_t subLength = task.right - task.left + 1;

        // case 1: portion is very small, use insertion sort
        if (subLength < 16) {
            isort(start + task.left, start + task.right + 1, comp);
            continue;
        }
        
        // case 2: depth limit reached use heap sort
        if (task.depth == 0) {
            hsort(start + task.left, start + task.right + 1, comp);
            continue;
        }

        // use median of three to find a pivot index for quick sort
        diff_t pivot_idx = detail::qsort_partition(start + task.left, start + task.right + 1, comp) - start;

        // add sub arrays tasks to the stack 
        // (small stack optimization) add first the largest sub array
        if (pivot_idx - 1 - task.left > task.right - (pivot_idx + 1)) {
            stack[++top] = {task.left, pivot_idx - 1, task.depth - 1};
            stack[++top] = {pivot_idx + 1, task.right, task.depth - 1};
        } else {
            stack[++top] = {pivot_idx + 1, task.right, task.depth - 1};
            stack[++top] = {task.left, pivot_idx - 1, task.depth - 1};
        }
    
    }


    
}

template <std::random_access_iterator Iterator>
JMAN_FORCE_INLINE constexpr void sort(Iterator first, Iterator last)
    noexcept(is_nothrow_sortable<Iterator>)
{
    sort(first, last, std::less{});
}

} // namespace jman