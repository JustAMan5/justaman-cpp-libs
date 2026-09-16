#pragma once

#include <justaman/utils/attributes.h>
#include <cstddef>
#include <utility>

namespace jman 
{

// #############################################################################################################
// ---------------------------------------------- CONSTRUCTING -----------------------------------------------//
// #############################################################################################################

/**
    @brief 
        wrapper around regular placement-new operator
*/
template <typename T, typename ...Args>
constexpr JMAN_FORCE_INLINE T* construct_at(T* ptr, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<T, Args...>) 
{
    return ::new(static_cast<void*>(ptr)) T{std::forward<Args>(args)...};
}


// #############################################################################################################
// ---------------------------------------------- DESTRUCTING -----------------------------------------------//
// #############################################################################################################

/**
    @brief 
        wrapper around regualar destructor call. Doesn't call destructor if its trivial.
*/
template <typename T>
constexpr JMAN_FORCE_INLINE void destruct_at(T* ptr)
    noexcept(std::is_nothrow_destructible_v<T>)
{
    if constexpr (!std::is_trivially_destructible_v<T>)
        ptr->~T();
}

/**
    @brief 
        Calls destructors (if non trivial) starting from [ptr] and ending at [ptr + @count - 1] moving forward
    
    @param count: amount of objects to destroy, the type for count is std::ptrdiff_t
*/
template <typename T>
constexpr JMAN_FORCE_INLINE void destruct_at(T* ptr, std::ptrdiff_t count)
    noexcept(std::is_nothrow_destructible_v<T>)
{
    if constexpr (std::is_trivially_destructible_v<T>)
        return;

    for (std::ptrdiff_t i = 0; i < count; ++i)
        (ptr + i)->~T();
}

/**
    @brief 
        Calls destructors (if non trivial) starting from [ptr + @count - 1] ending at [ptr] moving backwards
    
    @param count: amount of objects to destroy, the type for count is std::ptrdiff_t
*/
template <typename T>
constexpr JMAN_FORCE_INLINE void destruct_at(std::ptrdiff_t count, T* ptr)
    noexcept(std::is_nothrow_destructible_v<T>)
{
    if constexpr (std::is_trivially_destructible_v<T>)
        return;

    for (std::ptrdiff_t i = count; i-- > 0;)
        (ptr + i)->~T();
}

/**
    @brief 
        Calls destructors (if non trivial) starting from [ptr] and ending at [ptr + @count - 1] moving forward
    
    @param count: amount of objects to destroy, the type for count is std::size_t
*/
template <typename T>
constexpr JMAN_FORCE_INLINE void destruct_at(T* ptr, std::size_t count)
    noexcept(std::is_nothrow_destructible_v<T>)
{
    if constexpr (std::is_trivially_destructible_v<T>)
        return;

    for (std::size_t i = 0; i < count; ++i)
        (ptr + i)->~T();
}

/**
    @brief 
        Calls destructors (if non trivial) starting from [ptr + @count - 1] ending at [ptr] moving backwards
    
    @param count: amount of objects to destroy, the type for count is std::size_t
*/
template <typename T>
constexpr JMAN_FORCE_INLINE void destruct_at(std::size_t count, T* ptr)
    noexcept(std::is_nothrow_destructible_v<T>)
{
    if constexpr (std::is_trivially_destructible_v<T>)
        return;

    for (std::size_t i = count; i-- > 0;)
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
JMAN_NO_DISCARD constexpr JMAN_FORCE_INLINE T* addressof(T& x) noexcept
{
#   if defined (__has_builtin) && __has_builtin(__builtin_addressof)
        return __builtin_addressof(x);
#   else
        return reinterpret_cast<T*>(
            &const_cast<char&>(
                reinterpret_cast<const volatile char&>(x)
            )
        ); // dirty little trick that compilers used to bypass it, seems that is just making the compiler thing the object are raw bytes
#   endif
}
    
}; // namespace jman