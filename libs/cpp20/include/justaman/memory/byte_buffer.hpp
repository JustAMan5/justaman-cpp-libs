#pragma once

#include <justaman/utils/attributes.h>
#include <justaman/memory/endians.hpp>
#include <cassert>
#include <concepts>
#include <cstring>
#include <span>
#include <string_view>
#include <type_traits>

namespace jman 
{
    /**
        @brief
            Stack allocated byte buffer inspired from Roblox's buffer object class.
            Useful for networking or objects compressions into a single
        @note:
            This class preffers speed over safety, out of bounds checks r only checked at debug builds
            any out of bounds modification on release is undefined behavior.
    */
    template <std::size_t S>
    class byte_buffer 
    {   
        public:
            byte_buffer() noexcept = default;

            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t                     capacity() const noexcept { return S; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t                     size() const noexcept { return m_read; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t                     readable_bytes() const noexcept { return m_write - m_read; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::span<const std::byte, S>   view() const noexcept { return {m_buffer}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::span<std::byte, S>         view() noexcept { return {m_buffer}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::array<std::byte, S>        get() const noexcept { return {m_buffer}; }

            
            JMAN_FORCE_INLINE void                                      reset_indices() noexcept { m_read = m_write = 0; }
            JMAN_FORCE_INLINE void                                      reset_buffer() noexcept { reset_indices(); m_buffer = {}; }
            
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t               get_write() const noexcept { return m_write; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t               get_read() const noexcept { return m_read; }

            JMAN_FORCE_INLINE void                                      set_read(std::size_t byte) noexcept;
            JMAN_FORCE_INLINE void                                      set_write(std::size_t byte) noexcept;


            // ###############################################################
            // ---------------------------------- regulars integrlas / floats
            // #############################################################

            template <typename T> requires std::integral<T> || std::floating_point<T>
            void                                        write(T value) noexcept;
            template <typename T> requires std::integral<T> || std::floating_point<T>
            void                                        write(std::size_t byte, T value) noexcept;

            
            template <typename  T> requires std::integral<T> || std::floating_point<T>
            JMAN_NO_DISCARD T                           read() noexcept;
            template <typename  T> requires std::integral<T> || std::floating_point<T>
            JMAN_NO_DISCARD T                           read(std::size_t byte) const noexcept;

            // ###############################################################
            // ----------------------------------------------- little endian
            // #############################################################

            template <typename T> requires std::integral<T>
            void                                        write_le(T value) noexcept;
            template <typename T> requires std::integral<T>
            void                                        write_le(std::size_t byte, T value) noexcept;


            // ###############################################################
            // --------------------------------------------------- big endian
            // #############################################################

            template <typename T> requires std::integral<T>
            void                                        write_be(T value) noexcept;
            template <typename T> requires std::integral<T>
            void                                        write_be(std::size_t byte, T value) noexcept;

            
            // ###############################################################
            // -------------------------------------------------- raw strings
            // #############################################################

            void                                        write_string(std::string_view) noexcept;
            void                                        write_string(std::size_t byte, std::string_view) noexcept;

            JMAN_NO_DISCARD std::string_view            read_string(std::size_t byte, std::size_t size) const noexcept;
            JMAN_NO_DISCARD std::string_view            read_string(std::size_t size) noexcept;


            // ##############################################################
            // -------------------- prefixed size string (only for 8, 16, 32)
            // #############################################################

            template <std::unsigned_integral PS> // prefixed size (im not creative bruh)
            void                                        write_string(std::string_view) noexcept;
            template <std::unsigned_integral PS>
            void                                        write_string(std::size_t byte, std::string_view) noexcept;


            template <std::unsigned_integral PS>
            JMAN_NO_DISCARD std::string_view            read_string() noexcept;
            template <std::unsigned_integral PS>
            JMAN_NO_DISCARD std::string_view            read_string(std::size_t byte) const noexcept;

            // ##############################################################
            // -------------------------------------- raw bytes bia std:span
            // #############################################################

            template <typename U, std::size_t N>
            void                                        write_bytes(std::span<U, N> src) noexcept;
            template <typename U, std::size_t N>
            void                                        write_bytes(std::size_t byte, std::span<U, N> src) noexcept;

            template <typename U, std::size_t N>
            JMAN_NO_DISCARD std::span<U, N>             read_bytes() noexcept;
            template <typename U, std::size_t N>
            JMAN_NO_DISCARD std::span<U, N>             read_bytes(std::size_t byte) const noexcept;

        private:
            std::byte       m_buffer[S]     {};
            std::size_t     m_read          {0};
            std::size_t     m_write         {0};
    };
} // namespace jman

namespace jman 
{
// #################################################################################################
// ------------------------------------- WRITE / READ INDICES ------------------------------------//
// #################################################################################################

template <std::size_t S>
JMAN_FORCE_INLINE void byte_buffer<S>::set_read(std::size_t byte) noexcept {
    assert(byte >= capacity() && "jman::byte_buffer<S>::set_read() failed: byte param out of bounds.");
    m_read = byte;
}
template <std::size_t S>
JMAN_FORCE_INLINE void byte_buffer<S>::set_write(std::size_t byte) noexcept {
    assert(byte >= capacity() && "jman::byte_buffer<S>::set_write() failed: byte param out of bounds.");
    m_write = byte;
}

// #################################################################################################
// ------------------------------------- WRITING INTS, FLOATS ------------------------------------//
// #################################################################################################

template <std::size_t S>
template <typename  T> requires std::integral<T> || std::floating_point<T>
void byte_buffer<S>::write(T value) noexcept
{
    write(m_write, value);
    m_write += sizeof(T);
}
template <std::size_t S>
template <typename  T> requires std::integral<T> || std::floating_point<T>
void byte_buffer<S>::write(std::size_t byte, T value) noexcept
{
    assert(byte + sizeof(T) < capacity() && "jman::byte_buffer<S>::write() failed: @byte parameter out of bounds or value doesnt fit");
    std::memcpy(m_buffer + byte, &value, sizeof(T));
}

// #################################################################################################
// ------------------------------------- READING INTS, FLOATS ------------------------------------//
// #################################################################################################

template <std::size_t S>
template <typename T>  requires std::integral<T> || std::floating_point<T>
JMAN_NO_DISCARD T byte_buffer<S>::read() noexcept
{
    T value = read<T>(m_read);
    m_read += sizeof(T);
    return value;
}
template <std::size_t S>
template <typename T> requires std::integral<T> || std::floating_point<T>
JMAN_NO_DISCARD T byte_buffer<S>::read(std::size_t byte) const noexcept
{
    assert(byte + sizeof(T) < capacity() && "jman::byte_buffer<S>::read() failed: @byte parameter out of bounds");
    T value;
    std::memcpy(&value, m_buffer + byte, sizeof(T));
    return value;
}

// #################################################################################################
// -------------------------------------- WRITE LITTLE ENDIANS ------------------------------------//
// #################################################################################################

template <std::size_t S>
template <typename T> requires std::integral<T>
void byte_buffer<S>::write_le(T value) noexcept
{
    write_le(m_write, value);
    m_write += sizeof(T);
}
template <std::size_t S>
template <typename T> requires std::integral<T>
void byte_buffer<S>::write_le(std::size_t byte, T value) noexcept
{
    assert(byte + sizeof(T) < capacity() && "jman::byte_buffer<S>::write_le() failed: @byte parameter out of bounds or value doesnt fit");

    value = ensure_little_endian(value);
    std::memcpy(m_buffer + byte, &value, sizeof(T));
}


// #################################################################################################
// --------------------------------------- WRITE BIG ENDIANS--------------------------------------//
// #################################################################################################

template <std::size_t S>
template <typename T> requires std::integral<T>
void byte_buffer<S>::write_be(T value) noexcept
{
    write_le(m_write, value);
    m_write += sizeof(T);
}
template <std::size_t S>
template <typename T> requires std::integral<T>
void byte_buffer<S>::write_be(std::size_t byte, T value) noexcept
{
    assert(byte + sizeof(T) < capacity() && "jman::byte_buffer<S>::write_be() failed: @byte parameter out of bounds or value doesnt fit");

    value = ensure_big_endian(value);
    std::memcpy(m_buffer + byte, &value, sizeof(T));
}

// #################################################################################################
// --------------------------------------- WRTIE RAW STRINGS -------------------------------------//
// #################################################################################################

template <std::size_t S>
void byte_buffer<S>::write_string(std::string_view str) noexcept
{
    write_string(m_write, str);
    m_write += str.size();
}

template <std::size_t S>
void byte_buffer<S>::write_string(std::size_t byte, std::string_view str) noexcept
{
    assert(byte + str.size() < capacity() && "jman::byte_buffer<S>::write_string() failed: @byte parameter out of bounds or value doesnt fit");
    std::memcpy(m_buffer + byte, str.data(), str.size());
}

// #################################################################################################
// --------------------------------------- READ RAW STRINGS -------------------------------------//
// #################################################################################################


template <std::size_t S>
JMAN_NO_DISCARD std::string_view byte_buffer<S>::read_string(std::size_t size) noexcept
{
    std::string_view str = read_string(m_read, size);
    m_read += size;
    return str;
}

template <std::size_t S>
JMAN_NO_DISCARD std::string_view byte_buffer<S>::read_string(std::size_t byte, std::size_t size) const noexcept
{
    assert(byte + size < capacity() && "jman::byte_buffer<S>::read_string() failed: @byte or @size parameter out of bounds");
    return { reinterpret_cast<const char*>(m_buffer + byte), size };
}

// #################################################################################################
// ---------------------------------- WRTIE SIZE PREFIXED STRING ---------------------------------//
// #################################################################################################

template <std::size_t S>
template <std::unsigned_integral PS>
void byte_buffer<S>::write_string(std::string_view str) noexcept
{
    write_string(m_write, str);
    m_write += sizeof(PS) + str.size();
}

template <std::size_t S>
template <std::unsigned_integral PS>
void byte_buffer<S>::write_string(std::size_t byte, std::string_view str) noexcept
{
    static_assert(
        std::is_same_v<PS, std::uint8_t> || 
        std::is_same_v<PS, std::uint16_t> || 
        std::is_same_v<PS, std::uint32_t>,
    "PS typename must be of type uint8_t, uint16_t or uint32_t");
    assert(byte + sizeof(PS) + str.size() < capacity() && "jman::byte_buffer<S>::write_string<PS>() failed: @byte parameter out of bounds or value doesnt fit");

    PS size = str.size();

    std::memcpy(m_buffer + byte, &size, sizeof(PS));
    std::memcpy(m_buffer + byte + sizeof(PS), str.data(), str.size());
}

// #################################################################################################
// ---------------------------------- READ SIZE PREFIXED STRING ----------------------------------//
// #################################################################################################

template <std::size_t S>
template <std::unsigned_integral PS>
JMAN_NO_DISCARD std::string_view byte_buffer<S>::read_string() noexcept
{
    PS size;
    std::memcpy(&size, m_buffer + m_read, sizeof(PS));

    std::string_view str = read_string(m_read + sizeof(PS), size);
    m_read += sizeof(PS) + size;

    return str;
}
template <std::size_t S>
template <std::unsigned_integral PS>
JMAN_NO_DISCARD std::string_view byte_buffer<S>::read_string(std::size_t byte) const noexcept
{
    static_assert(
        std::is_same_v<PS, std::uint8_t> || 
        std::is_same_v<PS, std::uint16_t> || 
        std::is_same_v<PS, std::uint32_t>,
    "PS typename must be of type uint8_t, uint16_t or uint32_t");
    assert(byte + sizeof(PS) + size < capacity() && "jman::byte_buffer<S>::read_string<PS>() failed: @byte parameter out of bounds");

    PS size;
    std::memcpy(&size, m_buffer + byte, sizeof(PS));

    return { reinterpret_cast<const char*>(m_buffer + byte + sizeof(PS)), size };
}

// #################################################################################################
// -------------------------------------- WRITING RAW BYTES --------------------------------------//
// #################################################################################################

template <std::size_t S>
template <typename U, std::size_t N>
void byte_buffer<S>::write_bytes(std::span<U, N> src) noexcept
{
    write_bytes(m_write, src);
    m_write += src.size_bytes();
}
template <std::size_t S>
template <typename U, std::size_t N>
void byte_buffer<S>::write_bytes(std::size_t byte, std::span<U, N> src) noexcept
{
    assert(m_write + src.size_bytes() < capacity() && "jman::byte_buffer<S>::write_bytes() failed: @byte parameter out of bounds or value doesnt fit");
    std::memcpy(m_buffer + byte, src.data(), src.size_bytes());
}

// #################################################################################################
// -------------------------------------- READING RAW BYTES --------------------------------------//
// #################################################################################################

template <std::size_t S>
template <typename U, std::size_t N>
JMAN_NO_DISCARD std::span<U, N> byte_buffer<S>::read_bytes() noexcept
{   
    std::span<U, N> bytes = read_bytes(m_read);
    m_read += bytes.size_bytes();
    return bytes;
}
template <std::size_t S>
template <typename U, std::size_t N>
JMAN_NO_DISCARD std::span<U, N> byte_buffer<S>::read_bytes(std::size_t byte) const noexcept
{
    assert(m_write + sizeof(U) * N < capacity() && "jman::byte_buffer<S>::read_bytes() failed: @byte parameter out of bounds");
    return {m_buffer + byte, N};
}


} // namespace jman