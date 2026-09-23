#pragma once

#include "justaman/memory/lifetime.hpp"
#include "justaman/utils/attributes.h"
#include <type_traits>
#include <utility>

namespace jman 
{
    /**
        @brief 
            simpler version of https://github.com/joboccara/NamedType
    */
    template <typename T, typename Tag, template <typename> class... Skills>
    class strong_type: public Skills<strong_type<T, Tag, Skills...>>...
    {
        T m_value {};

        public:
            using argument = T;

            constexpr strong_type() noexcept(std::is_nothrow_default_constructible_v<T>) = default;

            template <typename ...Args>
            constexpr strong_type(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>): m_value(std::forward<Args>(args)...) {}

            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr T&          get() & noexcept { return m_value; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr const T&    get() const& noexcept { return m_value; } 
            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr T&&         get() && noexcept { return m_value; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr const T&&   get() const&& noexcept { return m_value; } 

            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr T&          operator*() & noexcept { return m_value; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr const T&    operator*() const& noexcept { return m_value; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr T&&         operator*() && noexcept { return m_value; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr const T&&   operator*() const&& noexcept { return m_value; }

            const T*                                                operator->() const noexcept { return jman::addressof(m_value); }
            T*                                                      operator->() noexcept { return jman::addressof(m_value); }

            using ref = strong_type<T&, Tag, Skills...>; // returns a strong_type to reference the current value
            constexpr operator ref() { return {m_value}; }
    };
} // namespace jman