#pragma once

#include <iterator>
#include <concepts>

namespace jman
{
// #####################################################################################################
// ---------------------------------------- LINEAR OPERATIONS ----------------------------------------//
// #####################################################################################################

// ###################################################
// ---------------------------------- FIND OPERATIONS
// ##################################################

template <std::input_iterator InputIt, typename T = std::iter_value_t<InputIt>>
requires std::equality_comparable_with<std::iter_value_t<InputIt>, T>
constexpr InputIt find_if(InputIt first, InputIt last, const T& value)
{
    for (; first != last; ++first)
        if (*first == value)
            return first;
    
    return last;
}

template <std::input_iterator InputIt, std::indirect_unary_predicate<InputIt> Pred>
constexpr InputIt find_if(InputIt first, InputIt last, Pred pred)
{
    for (; first != last; ++first)
        if (pred(*first))
            return first;

    return last;
}


// ###################################################
// -------------------------------------- QUANTIFIERS
// ##################################################

template <std::input_iterator InputIt, std::indirect_unary_predicate<InputIt> Pred>
constexpr bool any_of(InputIt first, InputIt last, Pred pred)
{   
    for (; first != last; ++first)
        if (pred(*first))
            return true;
    
    return false;
}




template <std::input_iterator InputIt, std::indirect_unary_predicate<InputIt> Pred>
constexpr bool all_of(InputIt first, InputIt last, Pred pred)
{
    for (; first != last; ++first)
        if (!pred(*first))
            return false;
    
    return true;
}

// ###################################################
// --------------------------------- COUNT OPERATIONS
// ##################################################


template <std::input_iterator InputIt, typename T = std::iter_value_t<InputIt>>
requires std::equality_comparable_with<std::iter_value_t<InputIt>, T>
constexpr auto count(InputIt first, InputIt last, const T& value)
{
    typename std::iter_difference_t<InputIt> ret {0};
    for (; first != last; ++first)
        if (*first == value)
            ++ret;
    return ret;
}

template <std::input_iterator InputIt, std::indirect_unary_predicate<InputIt> Pred>
constexpr auto count(InputIt first, InputIt last, Pred pred)
{
    typename std::iter_difference_t<InputIt> ret {0};
    for (; first != last; ++first)
        if (pred(*first))
            ++ret;
    return ret;
}


// ###################################################
// ---------------------------------------- ITERATION
// ##################################################

template <std::input_iterator InputIt, std::indirectly_unary_invocable<InputIt> Func>
constexpr Func for_each(InputIt first, InputIt last, Func f)
{
    for (; first != last; ++first)
        f(*first);
    
    return f;
}


} // namespace jman