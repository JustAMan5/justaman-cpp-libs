#pragma once

#include <justaman/utils/attributes.h>
#include <justaman/containers/vector.hpp> // forced for merge sort for the temp buffer
#include <bit>
#include <cstdint>
#include <iterator>
#include <type_traits>

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
    constexpr void msort(Iterator start, Iterator end, Compare comp); // no noexcept since it needs to allocate space for the temp vector for the copies

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

struct intro_sort_task 
{
    std::size_t left;
    std::size_t right;
    std::size_t depth;
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
constexpr void hsort_heapify(Iterator start, Iterator end, std::size_t i, Comp comp) 
    noexcept(jman::is_nothrow_sortable_comp<Iterator, Comp>)
{
    std::size_t length = end - start;
    
    while (true)
    {
        std::size_t largest = i;
        std::size_t l = 2 * i + 1;
        std::size_t r = 2 * i + 2;

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
    jman::vector<std::iter_value_t<Iterator>>* temp,
    Comp comp
)
    noexcept(jman::is_nothrow_sortable_comp<Iterator, Comp>)
{
    std::size_t i = 0;
    auto start1 = L1;
    auto start2 = L2;
    

    // in here we r using <= because the code treats R1 and R2 as the last actual element and not the next position after the last
    while (L1 <= R1 && L2 <= R2)
    {
        if (comp(*L1, *L2)) {
            (*temp)[i] = std::move_if_noexcept(*L1);
            ++i;
            ++L1;
        }
        else {
            (*temp)[i] = std::move_if_noexcept(*L2);
            ++i;
            ++L2;
        }
    }

    // these two r in case there is any element left that we cant compare with another

    while (L1 <= R1) 
    {
        (*temp)[i] = std::move_if_noexcept(*L1);
        ++i;
        ++L1;
    }

    while (L2 <= R2)
    {
        (*temp)[i] = std::move_if_noexcept(*L2);
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
    using T = std::iter_value_t<Iterator>;

    std::size_t length = end - start;
    if (length <= 1) return;

    jman::vector<T> tmp {}; tmp.resize(length);

    std::size_t len = 1;
    while (len < length)
    {
        std::size_t i = 0;
        while (i < length)
        {
            std::size_t L1 = i;
            std::size_t R1 = i + len - 1;
            std::size_t L2 = i + len;
            std::size_t R2 = i + 2 * len - 1;

            if (L2 >= length)
                break;

            if (R2 >= length)
                R2 = length - 1;

            detail::msort_merge(start + L1, start + R1, start + L2, start + R2, &tmp, comp);
            for (std::size_t j = 0; j < R2 - L1 + 1; ++j)
                start[i + j] = std::move_if_noexcept(tmp[j]);
    
            i += len << 1;

        }
        len <<= 1; // same as len *= 2
    }
}

template <typename Iterator>
requires std::random_access_iterator<Iterator>
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
    std::size_t length = end - start;
    if (length <= 1) return;

    // build heap inplace
    for (std::int64_t i = length / 2 - 1; i >= 0; --i)
        detail::hsort_heapify(start, end, i, comp);

    // one by one extract top elements from the heap and swap them with the back
    for (std::int64_t i = length - 1; i > 0; --i) {

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
    std::size_t length = end - start;
    if (length <= 1) return;

    using T = std::iter_value_t<Iterator>;

    // start with the second element since the first one is assumed its already sorted
    for (std::size_t i = 1; i < length; ++i) {
        T value = std::move_if_noexcept(start[i]);
        std::size_t j = i - 1;

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
    std::size_t length = end - start;
    if (length <= 1) return;

    for (std::size_t i = 0; i < length - 1; ++i) {

        std::size_t min_idx = i;

        // iterate through the unsorted portion
        // to find the actual minimum
        for (int j = i + 1; j < length; ++j)
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
    std::size_t length = end - start;
    if (length <= 1) return;

    std::size_t depthLimit = 2 * std::bit_width(length);

    detail::intro_sort_task stack[128];
    int top = -1;
    stack[++top] = {0, length - 1, depthLimit};


    while (top >= 0) 
    {
        detail::intro_sort_task task = stack[top--];
        std::size_t subLength = task.right - task.left + 1;

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
        std::size_t pivot_idx = detail::qsort_partition(start + task.left, start + task.right + 1, comp) - start;

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