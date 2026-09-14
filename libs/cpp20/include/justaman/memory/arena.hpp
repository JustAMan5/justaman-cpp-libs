#pragma once

#include <justaman/utils/attributes.h>
#include <cstddef>
#include <utility>

namespace jman 
{
    class arena 
    {
        public:
            explicit arena(std::size_t size);
            ~arena() noexcept;

            arena(const arena&) = delete;
            arena& operator=(const arena&) = delete;

            JMAN_NO_DISCARD inline std::byte*       buffer() const noexcept             { return m_buffer; }
            JMAN_NO_DISCARD inline std::size_t      size() const noexcept               { return m_size; }
            JMAN_NO_DISCARD inline std::size_t      offset() const noexcept             { return m_offset; }
            JMAN_NO_DISCARD inline std::size_t      available_space() const noexcept    { return m_size - m_offset; }

            void                                    reset() noexcept                    {m_offset = 0;};
            void*                                   allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t));
            
            template <typename T, typename ...Args>
            JMAN_NO_DISCARD T*                      allocate(Args&&... args) 
            {
                void* address = allocate(sizeof(T), alignof(T));
                if (address == nullptr) return nullptr;
                
                return ::new(address) T{ std::forward<Args>(args)... };
            }
        
        private:
            std::byte* m_buffer;
            std::size_t m_size;
            std::size_t m_offset;
    };

} // namespace jman
