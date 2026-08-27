#pragma once

#include <type_traits>

namespace jman 
{
    
    template <typename F, typename ...Args>
    struct is_callable 
    {
        static constexpr bool value = std::is_invocable_v<F, Args...>;
    };

    template<typename F, typename ...Args>
    inline constexpr bool is_callable_v = std::is_invocable_v<F, Args...>;
}