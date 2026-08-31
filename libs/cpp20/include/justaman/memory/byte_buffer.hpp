#pragma once

#include <cassert>
#include <concepts>
#include <cstring>
#include <span>
#include <string_view>
#include <justaman/utils/attributes.h>



namespace jman 
{
    /**
        @brief
            Stack allocated byte buffer inspired from Roblox's buffer object class.
            Useful for networking or objects compressions into a single
        @note:
            This class preffers speed over safety, out of bounds checks r only checked at debug builds
            any out of bounds modification on release is undefined behavior.
        @todo:
            add F16, F24, uint24_t and int24_t types and little endeinds values (useful for networking idk why)
    */
    template <std::size_t S>
    class byte_buffer 
    {   
        std::byte m_buffer[S] {};
        std::size_t m_read = 0;
        std::size_t m_write = 0;

        public:
            byte_buffer() noexcept = default;
            ~byte_buffer() noexcept = default;

            byte_buffer(const byte_buffer&) noexcept = default;
            byte_buffer& operator=(const byte_buffer&) noexcept = default;
            byte_buffer(byte_buffer&&) noexcept = default;
            byte_buffer& operator=(byte_buffer&&) noexcept = default;

            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t capacity() const noexcept { return S; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t size() const noexcept { return m_read; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t readable_bytes() const noexcept { return m_write - m_read; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::span<std::byte> view() const noexcept { return {&m_buffer, S}; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::array<std::byte, S> get() const noexcept { return {&m_buffer, S}; }

            JMAN_FORCE_INLINE void set_read(std::size_t byte) noexcept {
                assert(byte >= capacity() && "jman::byte_buffer<S>::set_read() failed: byte param out of bounds.");
                m_read = byte;
            }
            JMAN_FORCE_INLINE void set_write(std::size_t byte) noexcept {
                assert(byte >= capacity() && "jman::byte_buffer<S>::set_write() failed: byte param out of bounds.");
                m_write = byte;
            }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE  std::size_t get_read() const noexcept { return m_read; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE  std::size_t get_write() const noexcept { return m_write; }
            JMAN_FORCE_INLINE void reset_indices() noexcept { m_read = m_write = 0; }
            JMAN_FORCE_INLINE void reset_buffer() noexcept {reset_indices(); m_buffer = {}; }

            template <std::integral T>
            void write(T value) noexcept;
            template <std::floating_point T>
            void write(T value) noexcept;
            template <std::integral T>
            void write(std::size_t byte, T value) noexcept;
            template <std::floating_point T>
            void write(std::size_t byte, T value) noexcept;

            
            template <std::integral T>
            JMAN_NO_DISCARD T read() noexcept;
            template <std::floating_point T>
            JMAN_NO_DISCARD T read() noexcept;
            template <std::integral T>
            JMAN_NO_DISCARD T read(std::size_t byte) const noexcept;
            template <std::floating_point T>
            JMAN_NO_DISCARD T read(std::size_t byte) const noexcept;
            
            void write_string(std::string_view) noexcept;
            JMAN_NO_DISCARD std::string_view read_string(std::size_t size) noexcept;

            void write_string(std::size_t byte, std::string_view) noexcept;
            JMAN_NO_DISCARD std::string_view read_string(std::size_t byte, std::size_t size) const noexcept;
    };
}

// #################################################################################################
// ------------------------------------- WRITING INTS, FLOATS ------------------------------------//
// #################################################################################################

template <std::size_t S>
template <std::integral T>
void jman::byte_buffer<S>::write(T value) noexcept
{
    assert(m_write + sizeof(T) < capacity());

    *reinterpret_cast<T*>(&m_buffer[m_write]) = value;
    m_write += sizeof(T);
}
template <std::size_t S>
template <std::floating_point T>
void jman::byte_buffer<S>::write(T value) noexcept
{
    assert(m_write + sizeof(T) < capacity());

    *reinterpret_cast<T*>(&m_buffer[m_write]) = value;
    m_write += sizeof(T);
}
template <std::size_t S>
template <std::integral T>
void jman::byte_buffer<S>::write(std::size_t byte, T value) noexcept
{
    assert(byte + sizeof(T) < capacity());

    *reinterpret_cast<T*>(&m_buffer[byte]) = value;
}
template <std::size_t S>
template <std::floating_point T>
void jman::byte_buffer<S>::write(std::size_t byte, T value) noexcept
{
    assert(byte + sizeof(T) < capacity());

    *reinterpret_cast<T*>(&m_buffer[byte]) = value;
}

// #################################################################################################
// ------------------------------------- READING INTS, FLOATS ------------------------------------//
// #################################################################################################

template <std::size_t S>
template <std::integral T>
JMAN_NO_DISCARD T jman::byte_buffer<S>::read() noexcept
{
    assert(m_read + sizeof(T) < capacity());

    T value = *reinterpret_cast<T*>(&m_buffer[m_read]);
    m_read += sizeof(T);

    return value;
}
template <std::size_t S>
template <std::floating_point T>
JMAN_NO_DISCARD T jman::byte_buffer<S>::read() noexcept
{
    assert(m_read + sizeof(T) < capacity());

    T value = *reinterpret_cast<T*>(&m_buffer[m_read]);
    m_read += sizeof(T);

    return value;
}
template <std::size_t S>
template <std::integral T>
JMAN_NO_DISCARD T jman::byte_buffer<S>::read(std::size_t byte) const noexcept
{
    assert(m_read + sizeof(T) < capacity());

    return *reinterpret_cast<T*>(const_cast<std::byte*>(&m_buffer[byte]));
}
template <std::size_t S>
template <std::floating_point T>
JMAN_NO_DISCARD T jman::byte_buffer<S>::read(std::size_t byte) const noexcept
{
    assert(m_read + sizeof(T) < capacity());

    return *reinterpret_cast<T*>(const_cast<std::byte*>(&m_buffer[byte]));
}

// #################################################################################################
// ------------------------------------- READ / WRTIE STRINGS ------------------------------------//
// #################################################################################################

template <std::size_t S>
void jman::byte_buffer<S>::write_string(std::string_view str) noexcept
{
    std::size_t size = str.size();
    assert(m_write + size < capacity());

    std::strncpy(reinterpret_cast<char*>(&m_buffer[m_write]), str.data(), size);
    m_write += size;
}
template <std::size_t S>
JMAN_NO_DISCARD std::string_view jman::byte_buffer<S>::read_string(std::size_t size) noexcept
{
    assert(m_read + size < capacity());
    std::string_view str {reinterpret_cast<char*>(&m_buffer[m_write]), size};
    m_read += size;
    return str;
}

template <std::size_t S>
void jman::byte_buffer<S>::write_string(std::size_t byte, std::string_view str) noexcept
{
    std::size_t size = str.size();
    assert(byte + size < capacity());

    std::strncpy(reinterpret_cast<char*>(&m_buffer[byte]), str.data(), size);
}
template <std::size_t S>
JMAN_NO_DISCARD std::string_view jman::byte_buffer<S>::read_string(std::size_t byte, std::size_t size) const noexcept
{
    assert(byte + size < capacity());
    std::string_view str {reinterpret_cast<char*>(const_cast<std::byte*>(&m_buffer[byte])), size};
    return str;
}