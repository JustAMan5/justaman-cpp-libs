#pragma once

#include "justaman/utils/attributes.h"
#include <string_view>
#include <unordered_set>
#include <string>

namespace jman 
{
    class string_pool
    {
        std::unordered_set<std::string> m_set {};
        
        public:
            std::string_view    intern(const std::string& str);
            std::string_view    intern(std::string&& str);
            std::string_view    intern(std::string_view str);
            std::string_view    intern(const char* str);
            void                remove(const std::string& str);
            void                remove(std::string_view str);
            void                clear() noexcept;
    };

    class interned_string 
    {
        std::string_view m_view;

        public:
            JMAN_FORCE_INLINE static string_pool& pool() 
            {
                static string_pool pool;
                return pool;
            }

            explicit interned_string(std::string_view str) { m_view = pool().intern(std::string(str)); }
            explicit interned_string(const std::string& str) { m_view = pool().intern(str); }
            explicit interned_string(std::string&& str) { m_view = pool().intern(str); }
            explicit interned_string(const char* str) { m_view = pool().intern(str); }

            JMAN_FORCE_INLINE void  remove() noexcept { pool().remove(m_view); }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::string_view get() const noexcept { return m_view; }
            JMAN_FORCE_INLINE bool operator==(const interned_string& other) const noexcept { return m_view.data() == other.m_view.data(); }
            JMAN_FORCE_INLINE bool operator!=(const interned_string& other) const noexcept { return m_view.data() != other.m_view.data(); }
    };
}