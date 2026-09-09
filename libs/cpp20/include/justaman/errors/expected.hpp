#pragma once

#include <type_traits>

namespace jman 
{
    template <typename R, typename E>
    class excepted
    {
        static_assert(!std::is_void_v<E>, "Error result type cann't be of type void.");

        
    };
};