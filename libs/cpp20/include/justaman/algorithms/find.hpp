#pragma once

#include <iterator>
#include <concepts>

namespace jman
{

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

} // namespace jman