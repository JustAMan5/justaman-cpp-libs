#pragma once

#include "justaman/utils/attributes.h"
#if !(defined (__has_builtin) && __has_builtin(__builtin_mul_overflow))
    #include <limits> // for is_safe_mul
#endif

namespace jman
{
    template <typename T>
    JMAN_FORCE_INLINE const T& max(const T& a, const T& b)
    {
        return (a > b) ? a : b;
    }

    template <typename T>
    JMAN_FORCE_INLINE const T& min(const T& a, const T& b)
    {
        return (a < b) ? a : b;
    }

    template <typename T>
    JMAN_FORCE_INLINE const T& clamp(const T& x, const T& lo, const T& hi)
    {
        return max(lo, min(x, hi));
    }

    /**
        @returns true if its a safe multiplication, false if it will overflow the size_type
    */
    template <typename T>
    JMAN_FORCE_INLINE bool is_safe_mul(const T& a, const T& b)
    {
        if (a == 0 || b == 0) return false;
        #if defined (__has_builtin) && __has_builtin(__builtin_mul_overflow)
            T result; // unused
            return !__builtin_mul_overflow(a, b, &result);
        #else
            return a <= std::numeric_limits<T>::max() / b;
        #endif
    }
}