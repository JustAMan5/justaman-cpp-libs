#pragma once

#include <iterator>
#include <type_traits>

namespace jman
{
    // #####################################################################################################
    // ---------------------------------------- LINEAR OPERATIONS ----------------------------------------//
    // #####################################################################################################

    template <typename InputIt, class T = typename std::iterator_traits<InputIt>::value_type>
    requires std::input_iterator<InputIt> &&
             std::same_as<typename std::iterator_traits<InputIt>::value_type, T>
    constexpr InputIt find_if(InputIt first, InputIt last, const T& value);

    template <typename InputIt, class Pred>
    requires std::input_iterator<InputIt> && 
            std::is_invocable_r_v<bool, Pred, const typename std::iterator_traits<InputIt>::value_type&>
    constexpr InputIt find_if(InputIt first, InputIt last, Pred pred);




    template <typename InputIt, class Pred>
    requires std::input_iterator<InputIt> && 
            std::is_invocable_r_v<bool, Pred, const typename std::iterator_traits<InputIt>::value_type&>
    constexpr bool any_of(InputIt first, InputIt last, Pred pred);




    template <typename InputIt, class Pred>
    requires std::input_iterator<InputIt> && 
            std::is_invocable_r_v<bool, Pred, const typename std::iterator_traits<InputIt>::value_type&>
    bool all_of(InputIt first, InputIt last, Pred pred);



    template <typename InputIt, class T  = typename std::iterator_traits<InputIt>::value_type>
    requires std::input_iterator<InputIt> && 
             std::same_as<typename std::iterator_traits<InputIt>::value_type, T>
    constexpr typename std::iterator_traits<InputIt>::difference_type 
        count(InputIt first, InputIt last, const T& value);

    template <typename InputIt, class Pred>
    requires std::input_iterator<InputIt> && 
             std::is_invocable_r_v<bool, Pred, const typename std::iterator_traits<InputIt>::value_type&>
    constexpr std::iterator_traits<InputIt>::difference_type 
        count(InputIt first, InputIt last, Pred pred);



    
    template <typename InputIt, class Pred>
    requires std::input_iterator<InputIt> && 
             std::is_invocable_r_v<bool, Pred, const typename std::iterator_traits<InputIt>::value_type&>
    constexpr Pred for_each(InputIt first, InputIt last, Pred f);
} // namespace jman

// #####################################################################################################
// ---------------------------------------- LINEAR OPERATIONS ----------------------------------------//
// #####################################################################################################


template <typename InputIt, class T>
requires std::input_iterator<InputIt> &&
         std::same_as<typename std::iterator_traits<InputIt>::value_type, T>
constexpr InputIt jman::find_if(InputIt first, InputIt last, const T& value)
{
    for (; first != last; ++first)
        if (*first == value)
            return first;
    
    return last;
}

template <typename InputIt, class Pred>
requires std::input_iterator<InputIt> && 
            std::is_invocable_r_v<bool, Pred, const typename std::iterator_traits<InputIt>::value_type&>
constexpr InputIt jman::find_if(InputIt first, InputIt last, Pred pred)
{
    for (; first != last; ++first)
        if (pred(*first))
            return first;

    return last;
}

template <typename InputIt, class Pred>
requires std::input_iterator<InputIt> && 
        std::is_invocable_r_v<bool, Pred, const typename std::iterator_traits<InputIt>::value_type&>
constexpr bool jman::any_of(InputIt first, InputIt last, Pred pred)
{   
    for (; first != last; ++first)
        if (pred(*first))
            return true;
    
    return false;
}

template <typename InputIt, class Pred>
requires std::input_iterator<InputIt> && 
        std::is_invocable_r_v<bool, Pred, const typename std::iterator_traits<InputIt>::value_type&>
bool jman::all_of(InputIt first, InputIt last, Pred pred)
{
    for (; first != last; ++first)
        if (!pred(*first))
            return false;
    
    return true;
}




template <typename InputIt, class T>
requires std::input_iterator<InputIt> &&
         std::same_as<typename std::iterator_traits<InputIt>::value_type, T>
constexpr typename std::iterator_traits<InputIt>::difference_type 
    jman::count(InputIt first, InputIt last, const T& value)
{
    typename std::iterator_traits<InputIt>::difference_type ret = 0;
    for (; first != last; ++first)
        if (*first == value)
            ++ret;
    return ret;
}

template <typename InputIt, class Pred>
requires std::input_iterator<InputIt> && 
         std::is_invocable_r_v<bool, Pred, const typename std::iterator_traits<InputIt>::value_type&>
constexpr std::iterator_traits<InputIt>::difference_type 
    jman::count(InputIt first, InputIt last, Pred pred)
{
    typename std::iterator_traits<InputIt>::difference_type ret = 0;
    for (; first != last; ++first)
        if (pred(*first))
            ++ret;
    return ret;
}


    

template <typename InputIt, class Pred>
requires std::input_iterator<InputIt> && 
            std::is_invocable_r_v<bool, Pred, const typename std::iterator_traits<InputIt>::value_type&>
constexpr Pred jman::for_each(InputIt first, InputIt last, Pred f)
{
    for (; first != last; ++first)
        f(*first);
    
    return f;
}