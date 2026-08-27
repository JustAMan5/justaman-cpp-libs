#include <justaman/cryptography/hashing.hpp>
#include <bit>
#include <cstdint>


std::uint64_t jman::hash64_murmur3_mix(std::uint64_t h) noexcept
{
    h ^= h >> 30;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 27;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 31;
    return h;
}

std::uint32_t jman::hash32_murmur3_mix(std::uint32_t h) noexcept
{
    h ^= h >> 16;
    h *= 0x85ebca6bUL;
    h ^= h >> 13;
    h *= 0xc2b2ae35UL;
    h ^= h >> 17;
    return h;
}





std::uint64_t jman::hash64_fnv1a_mix(std::uint64_t h) noexcept
{
    static constexpr std::uint64_t FNV_PRIME_64 = 0x100000001b3ULL;
    static constexpr std::uint64_t FNV_OFFEST_BITS_64 = 0xcbf29ce484222325ULL;

    std::uint64_t hash = FNV_OFFEST_BITS_64;

    hash = (hash ^ (h & 0xFF)) * FNV_PRIME_64;
    hash = (hash ^ ((h >> 8) & 0xFF)) * FNV_PRIME_64;
    hash = (hash ^ ((h >> 16) & 0xFF)) * FNV_PRIME_64;
    hash = (hash ^ ((h >> 24) & 0xFF)) * FNV_PRIME_64;
    hash = (hash ^ ((h >> 32) & 0xFF)) * FNV_PRIME_64;
    hash = (hash ^ ((h >> 40) & 0xFF)) * FNV_PRIME_64;
    hash = (hash ^ ((h >> 48) & 0xFF)) * FNV_PRIME_64;
    hash = (hash ^ ((h >> 56) & 0xFF)) * FNV_PRIME_64;

    return hash;
}

std::uint32_t jman::hash32_fnv1a_mix(std::uint32_t h) noexcept
{
    static constexpr std::uint32_t FNV_PRIME_32 = 0x01000193UL;
    static constexpr std::uint32_t FNV_OFFEST_BITS_32 = 0x811C9DC5UL;

    std::uint32_t hash = FNV_OFFEST_BITS_32;

    hash = (hash ^ (h & 0xFF)) * FNV_PRIME_32;
    hash = (hash ^ ((h >> 8) & 0xFF)) * FNV_PRIME_32;
    hash = (hash ^ ((h >> 16) & 0xFF)) * FNV_PRIME_32;
    hash = (hash ^ ((h >> 24) & 0xFF)) * FNV_PRIME_32;

    return hash;
}






std::uint64_t jman::hash64_xxhash_mix(std::uint64_t h) noexcept
{
    static constexpr std::uint64_t PRIME64_2 = 1401430006053021787ULL;
    static constexpr std::uint64_t PRIME64_3 = 9439111559812942469ULL;

    h ^= h >> 33;
    h *= PRIME64_2;
    h ^= h >> 29;
    h *= PRIME64_3;
    h ^= h >> 32;
    return h;
}

std::uint32_t jman::hash32_xxhash_mix(std::uint32_t h) noexcept
{
    static constexpr std::uint32_t PRIME32_2 = 0x85ebca77UL;
    static constexpr std::uint32_t PRIME32_3 = 0xc2b2ae3dUL;

    h ^= h >> 17;
    h *= PRIME32_2;
    h ^= h >> 13;
    h *= PRIME32_3;
    h ^= h >> 16;
    return h;
}





std::uint64_t jman::hash64_split_mix(std::uint64_t h) noexcept
{
    h ^= h >> 30;
    h *= 0xbf58476d1ce4e5b9ULL;
    h ^= h >> 27;
    h *= 0x94d049bb133111ebULL;
    h ^= h >> 31;
    return h;
}

std::uint32_t jman::hash32_split_mix(std::uint32_t h) noexcept
{
    h ^= h >> 15;
    h *= 0x85ebda6bUL;
    h ^= h >> 13;
    h *= 0xc2b2ae35UL;
    h ^= h >> 16;
    return h;   
}






std::uint64_t jman::hash64_xorshift_rotate_mix(std::uint64_t h) noexcept
{
    h += std::rotl(h, 11);
    h ^= h >> 31;
    h += std::rotl(h, 17);
    h ^= h >> 23;
    h += std::rotl(h, 29);
    return h;
}

std::uint32_t jman::hash32_xorshift_rotate_mix(std::uint32_t h) noexcept
{
    h += std::rotl(h, 7);
    h ^= h >> 15;
    h += std::rotl(h, 13);
    h ^= h >> 9;
    h += std::rotl(h, 21);
    return h;
}





// #############################################################################
// ----------------- Daniel Lemire's fast-range reduction ----------------------
// #############################################################################



std::uint32_t jman::hash32_fast_range(std::uint32_t x, std::uint32_t n) noexcept {
    return (std::uint32_t)((std::uint64_t)x * n >> 32);
}


#if defined(__SIZEOF_INT128__) // __int128 available by compiler (GCC or Clang)

    std::uint64_t jman::hash64_fast_range(std::uint64_t x, std::uint64_t n) noexcept {
        return (std::uint64_t)((__uint128_t)x * n >> 64);
    }

#elif defined(_MSC_VER) && defined(_M_X64) // MSVC compiler doesnt provide __int128 but it does provide _umul128()

    #include <intrin.h>

    std::uint64_t jman::hash64_fast_range(std::uint64_t x, std::uint64_t n) noexcept {
        std::uint64_t high;
        _umul128(x, n, &high); // returns the low bits but we dont care abt them
        return high;
    }


#else // fallback impl when not available 128b mul

    std::uint64_t jman::hash64_fast_range(std::uint64_t x, std::uint64_t n) noexcept {
        return x % n;
    }

#endif // __SIZEOF_INT128__