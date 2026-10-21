#pragma once

#include <iterator>
#include <concepts>

namespace jman
{

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


} // namespace jman