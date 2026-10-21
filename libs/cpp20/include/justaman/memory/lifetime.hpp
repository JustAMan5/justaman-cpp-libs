#pragma once

#include <justaman/utils/attributes.h>
#include <type_traits>
#include <utility>
#if !(defined (__cpp_lib_constexpr_new) && __cpp_lib_constexpr_new >= 202406L)
#include <memory> // for std::construct_at fallback
#endif

namespace jman 
{

// #############################################################################################################
// ---------------------------------------------- CONSTRUCTION -----------------------------------------------//
// #############################################################################################################

/**
    @brief 
        wrapper around regular placement-new operator or std::construct_at when C++26 constexpr suppport is not available
*/
template <typename T, typename ...Args>
JMAN_FORCE_INLINE constexpr T* construct_at(T* ptr, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<T, Args...>) 
{
#if defined (__cpp_lib_constexpr_new) && __cpp_lib_constexpr_new >= 202406L
        // happy path! compiler supports constexpr placement new!
        if constexpr (std::is_array_v<T>)
            return ::new (static_cast<void*>(ptr)) T[1]();
        else
            return ::new(static_cast<void*>(ptr)) T{std::forward<Args>(args)...};
#else
        // sad path :(, compiler doesnt support placement new with constexpr
        return std::construct_at(ptr, std::forward<Args>(args)...);
#endif
}


// #############################################################################################################
// ---------------------------------------------- DESTRUCTION -----------------------------------------------//
// #############################################################################################################

/**
    @brief 
        wrapper around regualar destructor call. Doesn't call destructor if its trivial.
*/
template <typename T>
JMAN_FORCE_INLINE constexpr void destruct_at(T* ptr)
    noexcept(std::is_nothrow_destructible_v<T>)
{
    if constexpr (!std::is_trivially_destructible_v<T>)
        ptr->~T();
}

/**
    @brief 
        Calls destructors (if non trivial) starting from [ptr] and ending at [ptr + @count - 1] moving forward
    
    @param count: amount of objects to destroy
*/
template <typename T, typename SizeType>
JMAN_FORCE_INLINE constexpr void destruct_at(T* ptr, SizeType count)
    noexcept(std::is_nothrow_destructible_v<T>)
{
    static_assert(std::is_integral_v<SizeType>, "SizeType must be an integral type.");

    if constexpr (std::is_trivially_destructible_v<T>)
        return;

    for (SizeType i = 0; i < count; ++i)
        (ptr + i)->~T();
}

/**
    @brief 
        Calls destructors (if non trivial) starting from [ptr + @count - 1] ending at [ptr] moving backwards
    
    @param count: amount of objects to destroy
*/
template <typename T, typename SizeType>
JMAN_FORCE_INLINE constexpr void destruct_at(SizeType count, T* ptr)
    noexcept(std::is_nothrow_destructible_v<T>)
{
    static_assert(std::is_integral_v<SizeType>, "SizeType must be an integral type.");

    if constexpr (std::is_trivially_destructible_v<T>)
        return;

    for (SizeType i = count; i-- > 0;)
        (ptr + i)->~T();
}

// #############################################################################################################
// --------------------------------------------- TYPES ADDRESS -----------------------------------------------//
// #############################################################################################################

/**
    @brief 
        Grabs the real address of the specified object bypassing any overload for the & operator for that class
*/
template <class T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr T* addressof(T& x) noexcept
{
#   if defined (__has_builtin) && __has_builtin(__builtin_addressof)
        return __builtin_addressof(x);
#   else
        return std::addressof(x);
#   endif
}
    
}; // namespace jman