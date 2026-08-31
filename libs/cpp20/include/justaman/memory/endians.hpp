#pragma once

#include "justaman/utils/attributes.h"
#include <bit>
#include <cstdint>

namespace jman 
{
    JMAN_NO_DISCARD std::uint16_t ensure_little_endian(std::uint16_t x) noexcept;
    JMAN_NO_DISCARD std::uint32_t ensure_little_endian(std::uint32_t x) noexcept;
    JMAN_NO_DISCARD std::uint64_t ensure_little_endian(std::uint64_t x) noexcept;
    
    JMAN_NO_DISCARD std::uint16_t ensure_big_endian(std::uint16_t x) noexcept;
    JMAN_NO_DISCARD std::uint32_t ensure_big_endian(std::uint32_t x) noexcept;
    JMAN_NO_DISCARD std::uint64_t ensure_big_endian(std::uint64_t x) noexcept;

    JMAN_NO_DISCARD std::uint16_t ensure_native_endian(std::uint16_t x, std::endian source) noexcept;
    JMAN_NO_DISCARD std::uint32_t ensure_native_endian(std::uint32_t x, std::endian source) noexcept;
    JMAN_NO_DISCARD std::uint64_t ensure_native_endian(std::uint64_t x, std::endian source) noexcept;

    JMAN_NO_DISCARD std::int16_t ensure_little_endian(std::int16_t x) noexcept;
    JMAN_NO_DISCARD std::int32_t ensure_little_endian(std::int32_t x) noexcept;
    JMAN_NO_DISCARD std::int64_t ensure_little_endian(std::int64_t x) noexcept;
    
    JMAN_NO_DISCARD std::int16_t ensure_big_endian(std::int16_t x) noexcept;
    JMAN_NO_DISCARD std::int32_t ensure_big_endian(std::int32_t x) noexcept;
    JMAN_NO_DISCARD std::int64_t ensure_big_endian(std::int64_t x) noexcept;

    JMAN_NO_DISCARD std::int16_t ensure_native_endian(std::int16_t x, std::endian source) noexcept;
    JMAN_NO_DISCARD std::int32_t ensure_native_endian(std::int32_t x, std::endian source) noexcept;
    JMAN_NO_DISCARD std::int64_t ensure_native_endian(std::int64_t x, std::endian source) noexcept;
    // defined in the .cpp file ^


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

    // supported types for big endian
    template <>
    struct big_endian<std::int16_t>;
    template <>
    struct big_endian<std::int32_t>;
    template <>
    struct big_endian<std::int64_t>;
    

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
}

// ###################################################################################
// --------------------------------- Little endian ---------------------------------//
// ###################################################################################


template <>
struct jman::little_endian<std::uint16_t>
{
    public:
        little_endian() = default;
        little_endian(std::uint16_t x): m_value(ensure_little_endian(x)) {}

        operator std::uint16_t() { return m_value; }
    private:
        std::uint16_t m_value {0};
};
template <>
struct jman::little_endian<std::uint32_t>
{
    public:
        little_endian() = default;
        little_endian(std::uint32_t x): m_value(ensure_little_endian(x)) {}

        operator std::uint32_t() { return m_value; }
    private:
        std::uint32_t m_value {0};
};
template <>
struct jman::little_endian<std::uint64_t>
{
    public:
        little_endian() = default;
        little_endian(std::uint64_t x): m_value(ensure_little_endian(x)) {}

        operator std::uint64_t() { return m_value; }
    private:
        std::uint64_t m_value {0};
};

// ###################################################################################
// ----------------------------------- Big endian ----------------------------------//
// ###################################################################################

template <>
struct jman::big_endian<std::int16_t>
{
    public:
        big_endian() = default;
        big_endian(std::int16_t x): m_value(ensure_big_endian(x)) {}

        operator std::int16_t() { return m_value; }
    private:
        std::int16_t m_value {0};
};
template <>
struct jman::big_endian<std::int32_t>
{
    public:
        big_endian() = default;
        big_endian(std::int32_t x): m_value(ensure_big_endian(x)) {}

        operator std::int32_t() { return m_value; }
    private:
        std::int32_t m_value {0};
};
template <>
struct jman::big_endian<std::int64_t>
{
    public:
        big_endian() = default;
        big_endian(std::int64_t x): m_value(ensure_big_endian(x)) {}

        operator std::int64_t() { return m_value; }
    private:
        std::int64_t m_value {0};
};