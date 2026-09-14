#pragma once

#include <justaman/utils/attributes.h>
#if !(defined (__has_builtin) && __has_builtin(__builtin_mul_overflow)) || \
!(defined (__has_builtin) && __has_builtin(__builtin_add_overflow))
#   include <type_traits>
#   include <limits>
#endif

namespace jman
{


// ############################################################################################
// -------------------------------------- NUMERIC UTILITIES ---------------------------------//
// ############################################################################################

template <typename T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE 
constexpr const T& max(const T& a, const T& b)
{
    return (a > b) ? a : b;
}

template <typename T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE 
constexpr const T& min(const T& a, const T& b)
{
    return (a < b) ? a : b;
}




template <typename T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE 
constexpr const T& clamp(const T& x, const T& lo, const T& hi)
{
    return max(lo, min(x, hi));
}

// ############################################################################################
// ------------------------------------ OVERFLOW VALIDATION ---------------------------------//
// ############################################################################################


/**
    @brief checks if a multiplication between @a and @b overflow the template type specified
    @returns true if its a safe multiplication, false if it will overflow the size_type specified
*/
template <typename T>
JMAN_NO_DISCARD 
constexpr bool validate_mul(const T& a, const T& b, T& result)
{
    if (a == 0 || b == 0) return false;
    #if defined (__has_builtin) && __has_builtin(__builtin_mul_overflow)
        return !__builtin_mul_overflow(a, b, &result);
    #else
        result = a * b;
        if constexpr (std::is_unsigned_v<T>)
            return a <= std::numeric_limits<T>::max() / b;
        else
            if (a > 0) {
                if (b > 0) return a <= std::numeric_limits<T>::max() / b;    // pos * pos
                else return b >= std::numeric_limits<T>::min() / a;          // pos * neg
            } else {
                if (b > 0) return a >= std::numeric_limits<T>::min() / b;    // neg * pos
                else return b >= std::numeric_limits<T>::max() / a;          // neg * neg
            }
    #endif
}

/**
    @brief checks if an addition between @a and @b overflow the template type specified
    @returns true if its a safe addition, false if it will overflow the size_type specified
*/
template <typename T>
JMAN_NO_DISCARD 
constexpr bool validate_add(const T& a, const T& b, T& result)
{
    #if defined (__has_builtin) && __has_builtin(__builtin_add_overflow)
        return !__builtin_add_overflow(a, b, &result);
    #else
        result = a + b;
        if constexpr (std::is_unsigned_v<T>)
            return a <= std::numeric_limits<T>::max() - b;
        else
            if (b > 0)
                return a <= std::numeric_limits<T>::max() - b;
            else
                return a >= std::numeric_limits<T>::min() - b;
    #endif
}

} // namespace jman
