#pragma once

#include "justaman/utils/attributes.h"
#include <bit>
#include <cstdint>

namespace jman 
{
    template <std::integral T>
    JMAN_NO_DISCARD JMAN_FORCE_INLINE T ensure_little_endian(T x) noexcept;

    template <std::integral T>
    JMAN_NO_DISCARD JMAN_FORCE_INLINE T ensure_big_endian(T x) noexcept;

    template <std::integral T>
    JMAN_NO_DISCARD JMAN_FORCE_INLINE T ensure_native_endian(T x, std::endian source) noexcept;



    template <typename T>
    struct little_endian 
    {
        static_assert(false, "Integral type not supported.");
    };

    template <typename T>
    struct big_endian 
    {
        static_assert(false, "Integral type not supported.");
    };

    // supported types for little endian
    template <>
    struct little_endian<std::uint16_t>;
    template <>
    struct little_endian<std::uint32_t>;
    template <>
    struct little_endian<std::uint64_t>;

    template <>
    struct little_endian<std::int16_t>;
    template <>
    struct little_endian<std::int32_t>;
    template <>
    struct little_endian<std::int64_t>;

    // supported types for big endian

    template <>
    struct big_endian<std::uint16_t>;
    template <>
    struct big_endian<std::uint32_t>;
    template <>
    struct big_endian<std::uint64_t>;

    template <>
    struct big_endian<std::int16_t>;
    template <>
    struct big_endian<std::int32_t>;
    template <>
    struct big_endian<std::int64_t>;

    // shortcuts for types
    

    using le_u16_t = little_endian<std::uint16_t>;
    using le_u32_t = little_endian<std::uint32_t>;
    using le_u64_t = little_endian<std::uint64_t>;

    using le_i16_t = little_endian<std::int16_t>;
    using le_i32_t = little_endian<std::int32_t>;
    using le_i64_t = little_endian<std::int64_t>;



    using be_u16_t = big_endian<std::uint16_t>;
    using be_u32_t = big_endian<std::uint32_t>;
    using be_u64_t = big_endian<std::uint64_t>;

    using be_i16_t = big_endian<std::int16_t>;
    using be_i32_t = big_endian<std::int32_t>;
    using be_i64_t = big_endian<std::int64_t>;
} // namespace jman

namespace jman::detail
{
    JMAN_NO_DISCARD std::uint16_t ensure_little_endian(std::uint16_t x) noexcept;
    JMAN_NO_DISCARD std::uint32_t ensure_little_endian(std::uint32_t x) noexcept;
    JMAN_NO_DISCARD std::uint64_t ensure_little_endian(std::uint64_t x) noexcept;

    JMAN_NO_DISCARD std::int16_t ensure_little_endian(std::int16_t x) noexcept;
    JMAN_NO_DISCARD std::int32_t ensure_little_endian(std::int32_t x) noexcept;
    JMAN_NO_DISCARD std::int64_t ensure_little_endian(std::int64_t x) noexcept;



    JMAN_NO_DISCARD std::uint16_t ensure_big_endian(std::uint16_t x) noexcept;
    JMAN_NO_DISCARD std::uint32_t ensure_big_endian(std::uint32_t x) noexcept;
    JMAN_NO_DISCARD std::uint64_t ensure_big_endian(std::uint64_t x) noexcept;
    
    JMAN_NO_DISCARD std::int16_t ensure_big_endian(std::int16_t x) noexcept;
    JMAN_NO_DISCARD std::int32_t ensure_big_endian(std::int32_t x) noexcept;
    JMAN_NO_DISCARD std::int64_t ensure_big_endian(std::int64_t x) noexcept;

    

    JMAN_NO_DISCARD std::uint16_t ensure_native_endian(std::uint16_t x, std::endian source) noexcept;
    JMAN_NO_DISCARD std::uint32_t ensure_native_endian(std::uint32_t x, std::endian source) noexcept;
    JMAN_NO_DISCARD std::uint64_t ensure_native_endian(std::uint64_t x, std::endian source) noexcept;

    JMAN_NO_DISCARD std::int16_t ensure_native_endian(std::int16_t x, std::endian source) noexcept;
    JMAN_NO_DISCARD std::int32_t ensure_native_endian(std::int32_t x, std::endian source) noexcept;
    JMAN_NO_DISCARD std::int64_t ensure_native_endian(std::int64_t x, std::endian source) noexcept;
    // defined in the .cpp file ^
} // namespace jman::detail

namespace jman 
{


// ###################################################################################
// --------------------------------- Little endian ---------------------------------//
// ###################################################################################


template <>
struct little_endian<std::uint16_t>
{
    public:
        little_endian() noexcept = default;
        template <std::integral T>
        little_endian(T x) noexcept: m_value(ensure_little_endian<std::uint16_t>(static_cast<std::uint16_t>(x))) {}

        template <std::integral T>
        operator T() const noexcept { return static_cast<T>(m_value); }
    private:
        std::uint16_t m_value {0};
};
template <>
struct little_endian<std::uint32_t>
{
    public:
        little_endian() = default;
        template <std::integral T>
        little_endian(T x) noexcept: m_value(ensure_little_endian<std::uint32_t>(static_cast<std::uint32_t>(x))) {}

        template <std::integral T>
        operator T() const noexcept { return static_cast<T>(m_value); }
    private:
        std::uint32_t m_value {0};
};
template <>
struct little_endian<std::uint64_t>
{
    public:
        little_endian() = default;
        template <std::integral T>
        little_endian(T x) noexcept: m_value(ensure_little_endian<std::uint64_t>(static_cast<std::uint64_t>(x))) {}

        template <std::integral T>
        operator T() const noexcept { return static_cast<T>(m_value); }
    private:
        std::uint64_t m_value {0};
};


template <>
struct little_endian<std::int16_t>
{
    public:
        little_endian() = default;
        template <std::integral T>
        little_endian(T x) noexcept: m_value(ensure_little_endian<std::int16_t>(static_cast<std::int16_t>(x))) {}

        template <std::integral T>
        operator T() const noexcept { return static_cast<T>(m_value); }
    private:
        std::int16_t m_value {0};
};
template <>
struct little_endian<std::int32_t>
{
    public:
        little_endian() = default;
        template <std::integral T>
        little_endian(T x) noexcept: m_value(ensure_little_endian<std::int32_t>(static_cast<std::int32_t>(x))) {}

        template <std::integral T>
        operator T() const noexcept { return static_cast<T>(m_value); }
    private:
        std::int32_t m_value {0};
};
template <>
struct little_endian<std::int64_t>
{
    public:
        little_endian() = default;
        template <std::integral T>
        little_endian(T x) noexcept: m_value(ensure_little_endian<std::int64_t>(static_cast<std::int64_t>(x))) {}

        template <std::integral T>
        operator T() const noexcept { return static_cast<T>(m_value); }
    private:
        std::int64_t m_value {0};
};

// ###################################################################################
// ----------------------------------- Big endian ----------------------------------//
// ###################################################################################

template <>
struct big_endian<std::uint16_t>
{
    public:
        big_endian() = default;
        template <std::integral T>
        big_endian(T x) noexcept: m_value(ensure_big_endian<std::uint16_t>(static_cast<std::uint16_t>(x))) {}

        template <std::integral T>
        operator T() const noexcept { return static_cast<T>(m_value); }
    private:
        std::uint16_t m_value {0};
};
template <>
struct big_endian<std::uint32_t>
{
    public:
        big_endian() = default;
        template <std::integral T>
        big_endian(T x) noexcept: m_value(ensure_big_endian<std::uint32_t>(static_cast<std::uint32_t>(x))) {}

        template <std::integral T>
        operator T() const noexcept { return static_cast<T>(m_value); }
    private:
        std::uint32_t m_value {0};
};
template <>
struct big_endian<std::uint64_t>
{
    public:
        big_endian() = default;
        template <std::integral T>
        big_endian(T x) noexcept: m_value(ensure_big_endian<std::uint64_t>(static_cast<std::uint64_t>(x))) {}

        template <std::integral T>
        operator T() const noexcept { return static_cast<T>(m_value); }
    private:
        std::uint64_t m_value {0};
};

template <>
struct big_endian<std::int16_t>
{
    public:
        big_endian() noexcept = default;
        template <std::integral T>
        big_endian(T x) noexcept: m_value(ensure_big_endian<std::int16_t>(static_cast<std::int16_t>(x))) {}

        template <std::integral T>
        operator T() const noexcept { return static_cast<T>(m_value); }
    private:
        std::int16_t m_value {0};
};
template <>
struct big_endian<std::int32_t>
{
    public:
        big_endian() = default;
        template <std::integral T>
        big_endian(T x) noexcept: m_value(ensure_big_endian<std::int32_t>(static_cast<std::int32_t>(x))) {}

        template <std::integral T>
        operator T() const noexcept { return static_cast<T>(m_value); }
    private:
        std::int32_t m_value {0};
};
template <>
struct big_endian<std::int64_t>
{
    public:
        big_endian() = default;
        template <std::integral T>
        big_endian(T x) noexcept: m_value(ensure_big_endian<std::int64_t>(static_cast<std::int64_t>(x))) {}

        template <std::integral T>
        operator T() const noexcept { return static_cast<T>(m_value); }
    private:
        std::int64_t m_value {0};
};

// ###################################################################################
// ------------------------------- ENDIAN FUNCTIONS --------------------------------//
// ###################################################################################

/**
    @brief returns a new @x but forced to be on the little endian signature 
    @note if hardware is little endian already it wont do anything
*/
template <std::integral T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE T ensure_little_endian(T x) noexcept
{
    static_assert(
        std::is_same_v<T, std::int16_t> ||
        std::is_same_v<T, std::int32_t> ||
        std::is_same_v<T, std::int64_t> ||
        std::is_same_v<T, std::uint16_t> ||
        std::is_same_v<T, std::uint32_t> ||
        std::is_same_v<T, std::uint64_t>, 
        "T typename must be a type of uint16_t, uint32_t, uint64_t or any of it's signed versions."
    );
    return detail::ensure_little_endian(x);
}

/**
    @brief returns a new @x but forced to be on the big endian signature 
    @note if hardware is big endian already it wont do anything
*/
template <std::integral T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE T ensure_big_endian(T x) noexcept
{
    static_assert(
        std::is_same_v<T, std::int16_t> ||
        std::is_same_v<T, std::int32_t> ||
        std::is_same_v<T, std::int64_t> ||
        std::is_same_v<T, std::uint16_t> ||
        std::is_same_v<T, std::uint32_t> ||
        std::is_same_v<T, std::uint64_t>, 
        "T typename must be a type of uint16_t, uint32_t, uint64_t or any of it's signed versions."
    );
    return detail::ensure_big_endian(x);
}

/**
    @brief returns a new @x transformed to the current computer native endian signature depending on the @source signature
    @note if hardware is already the source of @x it wont do anything
*/
template <std::integral T>
JMAN_NO_DISCARD JMAN_FORCE_INLINE T ensure_native_endian(T x, std::endian source) noexcept
{
    static_assert(
        std::is_same_v<T, std::int16_t> ||
        std::is_same_v<T, std::int32_t> ||
        std::is_same_v<T, std::int64_t> ||
        std::is_same_v<T, std::uint16_t> ||
        std::is_same_v<T, std::uint32_t> ||
        std::is_same_v<T, std::uint64_t>, 
        "T typename must be a type of uint16_t, uint32_t, uint64_t or any of it's signed versions."
    );
    return detail::ensure_native_endian(x, source);
}

} // namespace jman