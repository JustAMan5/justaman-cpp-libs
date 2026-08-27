#include <justaman/memory/arena.hpp>
#include <cstddef>
#include <cstdint>

// ------------------------ CONSTRUCTORS -----------------------//

jman::arena::arena(std::size_t size)
    :m_size(size), m_offset(0) 
{
    m_buffer = reinterpret_cast<std::byte*>(::operator new(size));
}

jman::arena::~arena() noexcept
{
    ::operator delete(m_buffer); 
}

// ------------------------- METHODS --------------------------//

void* jman::arena::allocate(std::size_t size, std::size_t alignment)
{
    std::uintptr_t current = reinterpret_cast<std::uintptr_t>(m_buffer + m_offset);
    std::uintptr_t aligned = (current + alignment - 1) & ~(alignment - 1); // formula for next alignment adress point
    std::size_t padding = aligned - current;

    if (m_offset + padding + size > m_size) return nullptr;

    m_offset += padding;
    void* result = m_buffer + m_offset;
    m_offset += size;
    return result;
}

void jman::arena::reset() noexcept
{
    m_offset = 0;
}
