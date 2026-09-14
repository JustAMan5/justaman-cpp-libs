#include <justaman/memory/endians.hpp>
#include <bit>
#include <concepts>

// ###################################################################################
// ------------------------------ Fallback swap impl -------------------------------//
// ###################################################################################

namespace jman::detail 
{
    template <std::unsigned_integral T>
    JMAN_FORCE_INLINE constexpr T bswap_fallback(T val) noexcept 
    {
        if constexpr (sizeof(T) == 1)
            return val;
        else if constexpr (sizeof(T) == 2)
            return (val >> 8) | (val << 8);
        else if constexpr (sizeof(T) == 4)
            return ((val >> 24) & 0x000000FFU) |
                   ((val >>  8) & 0x0000FF00U) |
                   ((val <<  8) & 0x00FF0000U) |
                   ((val << 24) & 0xFF000000U);
        else if constexpr (sizeof(T) == 8)
            return ((val >> 56) & 0x00000000000000FFULL) |
                   ((val >> 40) & 0x000000000000FF00ULL) |
                   ((val >> 24) & 0x0000000000FF0000ULL) |
                   ((val >>  8) & 0x00000000FF000000ULL) |
                   ((val <<  8) & 0x000000FF00000000ULL) |
                   ((val << 24) & 0x0000FF0000000000ULL) |
                   ((val << 40) & 0x00FF000000000000ULL) |
                   ((val << 56) & 0xFF00000000000000ULL);
    }
} // namespace jman::detail

// ###################################################################################
// ------------------------------- Global func util -------------------------------//
// ###################################################################################
#include <version>
#if !(defined (__cpp_lib_byteswap) && __cpp_lib_byteswap >= 202110L)
    #if defined(_MSC_VER)
        #include <stdlib.h>
    #elif defined(__APPLE__)
        #include <libkern/OSByteOrder.h>
    #elif defined(__linux__)
        #include <byteswap.h>
    #endif    
#endif // __cpp_lib_byteswap


namespace jman::detail 
{

template <std::integral T>
JMAN_FORCE_INLINE constexpr T bswap(T val) noexcept 
{
    using U = std::make_unsigned_t<T>;
    U u_val = static_cast<U>(val); // not safe to do the swap with signed integers types, so we convert it to signed and cast it later
    U result;

    #if defined (__cpp_lib_byteswap) && __cpp_lib_byteswap >= 202110L
        result = std::byteswap(u_val);
    #elif defined(_MSC_VER)
        if constexpr (sizeof(T) == 2) result = _byteswap_ushort(u_val);
        else if constexpr (sizeof(T) == 4) result = _byteswap_ulong(u_val);
        else if constexpr (sizeof(T) == 8) result = _byteswap_uint64(u_val);
    #elif defined(__APPLE__)
        if constexpr (sizeof(T) == 2) result = _OSSwapInt16(u_val);
        else if constexpr (sizeof(T) == 4) result = _OSSwapInt32(u_val);
        else if constexpr (sizeof(T) == 8) result = _OSSwapInt64(u_val);
    #elif defined(__GNUC__) || defined(__clang__)
        if constexpr (sizeof(T) == 2) result = __builtin_bswap16(u_val);
        else if constexpr (sizeof(T) == 4) result = __builtin_bswap32(u_val);
        else if constexpr (sizeof(T) == 8) result = __builtin_bswap64(u_val);
    #elif defined(__linux__)
        if constexpr (sizeof(T) == 2) result = bswap_16(u_val);
        else if constexpr (sizeof(T) == 4) result = bswap_32(u_val);
        else if constexpr (sizeof(T) == 8) result = bswap_64(u_val);
    #else
        result = bswap_fallback(u_val);
    #endif

    return static_cast<T>(result); // cast to the original type
}

// ###################################################################################
// ---------------------------------- Little endian --------------------------------//
// ###################################################################################

JMAN_NO_DISCARD std::uint16_t ensure_little_endian(std::uint16_t x) noexcept
{
    if constexpr (std::endian::native == std::endian::big) 
        return detail::bswap(x);
    return x;
}
JMAN_NO_DISCARD std::uint32_t ensure_little_endian(std::uint32_t x) noexcept
{
    if constexpr (std::endian::native == std::endian::big) 
        return detail::bswap(x);
    return x;
}
JMAN_NO_DISCARD std::uint64_t ensure_little_endian(std::uint64_t x) noexcept
{
    if constexpr (std::endian::native == std::endian::big) 
        return detail::bswap(x);
    return x;
}   


JMAN_NO_DISCARD std::int16_t ensure_little_endian(std::int16_t x) noexcept
{
    if constexpr (std::endian::native == std::endian::big) 
        return detail::bswap(x);
    return x;
}
JMAN_NO_DISCARD std::int32_t ensure_little_endian(std::int32_t x) noexcept
{
    if constexpr (std::endian::native == std::endian::big) 
        return detail::bswap(x);
    return x;
}
JMAN_NO_DISCARD std::int64_t ensure_little_endian(std::int64_t x) noexcept
{
    if constexpr (std::endian::native == std::endian::big) 
        return detail::bswap(x);
    return x;
}   

// ###################################################################################
// ------------------------------------ Big endian ---------------------------------//
// ###################################################################################



JMAN_NO_DISCARD std::uint16_t ensure_big_endian(std::uint16_t x) noexcept
{
    if constexpr (std::endian::native == std::endian::little) 
        return detail::bswap(x);
    return x;
}
JMAN_NO_DISCARD std::uint32_t ensure_big_endian(std::uint32_t x) noexcept
{
    if constexpr (std::endian::native == std::endian::little) 
        return detail::bswap(x);
    return x;
}
JMAN_NO_DISCARD std::uint64_t ensure_big_endian(std::uint64_t x) noexcept
{
    if constexpr (std::endian::native == std::endian::little) 
        return detail::bswap(x);
    return x;
}



JMAN_NO_DISCARD std::int16_t ensure_big_endian(std::int16_t x) noexcept
{
    if constexpr (std::endian::native == std::endian::little) 
        return detail::bswap(x);
    return x;
}
JMAN_NO_DISCARD std::int32_t ensure_big_endian(std::int32_t x) noexcept
{
    if constexpr (std::endian::native == std::endian::little) 
        return detail::bswap(x);
    return x;
}
JMAN_NO_DISCARD std::int64_t ensure_big_endian(std::int64_t x) noexcept
{
    if constexpr (std::endian::native == std::endian::little) 
        return detail::bswap(x);
    return x;
} 

// ###################################################################################
// ---------------------------------- Native endian --------------------------------//
// ###################################################################################


JMAN_NO_DISCARD std::uint16_t ensure_native_endian(std::uint16_t x, std::endian source) noexcept
{
    if (std::endian::native != source)
        return detail::bswap(x);
    return x;
}
JMAN_NO_DISCARD std::uint32_t ensure_native_endian(std::uint32_t x, std::endian source) noexcept
{
    if (std::endian::native != source)
        return detail::bswap(x);
    return x;
}
JMAN_NO_DISCARD std::uint64_t ensure_native_endian(std::uint64_t x, std::endian source) noexcept
{
    if (std::endian::native != source)
        return detail::bswap(x);
    return x;
}


JMAN_NO_DISCARD std::int16_t ensure_native_endian(std::int16_t x, std::endian source) noexcept
{
    if (std::endian::native != source)
        return detail::bswap(x);
    return x;
}
JMAN_NO_DISCARD std::int32_t ensure_native_endian(std::int32_t x, std::endian source) noexcept
{
    if (std::endian::native != source)
        return detail::bswap(x);
    return x;
}
JMAN_NO_DISCARD std::int64_t ensure_native_endian(std::int64_t x, std::endian source) noexcept
{
    if (std::endian::native != source)
        return detail::bswap(x);
    return x;
} 

} // namespace jman::detail