#pragma once

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
            strong_type() noexcept(std::is_nothrow_default_constructible_v<T>) = default;

            constexpr explicit strong_type(const T& v) noexcept(std::is_nothrow_copy_constructible_v<T>): m_value(v) {}
            constexpr explicit strong_type(T&& v) noexcept(std::is_nothrow_move_constructible_v<T>): m_value(std::move(v)) {}

            JMAN_NO_DISCARD constexpr T&                                    get() noexcept { return m_value; } // get as rvalue or lvalue depending on ref collapsing
            JMAN_NO_DISCARD constexpr const std::remove_reference_t<T>&     get() const noexcept { return m_value; } // view

            JMAN_NO_DISCARD constexpr T&                                    operator*() noexcept { return m_value; } // get as rvalue or lvalue depending on ref collapsing
            JMAN_NO_DISCARD constexpr const std::remove_reference_t<T>&     operator*() const noexcept { return m_value; } // view

            using ref = strong_type<T&, Tag, Skills...>; // returns a strong_type to reference the current value
            operator ref() { return {m_value}; }
    };
};