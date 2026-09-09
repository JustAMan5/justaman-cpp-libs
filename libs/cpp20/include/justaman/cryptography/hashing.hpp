#pragma once

#include <cstdint>

namespace jman 
{
    std::uint64_t   hash64_murmur3_mix(std::uint64_t h) noexcept;
    std::uint32_t   hash32_murmur3_mix(std::uint32_t h) noexcept;

    std::uint32_t   hash32_fnv1a_mix(std::uint32_t h) noexcept;
    std::uint64_t   hash64_fnv1a_mix(std::uint64_t h) noexcept;

    std::uint32_t   hash32_xxhash_mix(std::uint32_t h) noexcept;
    std::uint64_t   hash64_xxhash_mix(std::uint64_t h) noexcept;

    std::uint32_t   hash32_split_mix(std::uint32_t h) noexcept;
    std::uint64_t   hash64_split_mix(std::uint64_t h) noexcept;

    std::uint32_t   hash32_xorshift_rotate_mix(std::uint32_t h) noexcept;
    std::uint64_t   hash64_xorshift_rotate_mix(std::uint64_t h) noexcept;

    // ############################################################################################################
    // ------------------------------------- Daniel Lemire's fast-range reduction -------------------------------
    // ############################################################################################################

    /**
        @brief
            Daniel Lemire's fast-range reduction: a cool trick to doing x % y but with a almost identical speed to x & y
            https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
            \n   
            the CPU will compute the multiplication as uint128 and store the top and bottom bits in different 64 bits registers 
            (google ai told me that since we r just using the top bits the compiler can easily optimize that)
    */
    ///@{
    std::uint64_t   hash64_fast_range(std::uint64_t x, std::uint64_t n) noexcept;
    std::uint32_t   hash32_fast_range(std::uint32_t x, std::uint32_t n) noexcept;
    ///@}
}