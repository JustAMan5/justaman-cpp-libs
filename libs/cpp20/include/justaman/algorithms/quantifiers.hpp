#pragma once

#include <iterator>

namespace jman
{

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


} // namespace jman