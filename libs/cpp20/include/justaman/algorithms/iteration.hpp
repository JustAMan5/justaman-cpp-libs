#pragma once

#include <iterator>

namespace jman 
{

template <std::input_iterator InputIt, std::indirectly_unary_invocable<InputIt> Func>
constexpr Func for_each(InputIt first, InputIt last, Func f)
{
    for (; first != last; ++first)
        f(*first);
    
    return f;
}

} // namespace jman