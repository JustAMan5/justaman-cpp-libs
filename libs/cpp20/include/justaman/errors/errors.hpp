#pragma once

#include <justaman/utils/attributes.h>
#include <justaman/memory/lifetime.hpp>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace jman 
{
    using error_id = std::uint32_t;

    struct failure
    {
        public:
            constexpr failure(error_id id) noexcept: m_id(id) {};
            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr error_id id() const noexcept { return m_id; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr bool operator==(const failure&) const noexcept = default;

        private:
            error_id m_id;
    };

    struct context 
    {

    };

    template <typename T>
    class JMAN_NO_DISCARD result 
    {
        static_assert(!std::is_reference_v<T>);
        static_assert(!std::is_function_v<T>);
        static_assert(!std::is_same_v<std::remove_cv_t<T>, std::in_place_t>);

        public:
            using value_type = T;
            using error_type = error_id;

            // ###################################################
            // ------------------------------------- CONSTRUCTORS
            // #################################################

            template<class U> requires std::is_convertible_v<U, T>
            result(const result<U>& other);
            template<class U> requires std::is_convertible_v<U, T>
            result(result<U>&& other);

            template<class U = std::remove_cv_t<T>> requires std::is_convertible_v<U, T>
            result(U&& v): m_has_value(true), m_value(std::forward<U>(v)) {}

            template <class... Args> requires std::is_constructible_v<T, Args...>
            result(std::in_place_t, Args&&... args): m_has_value(true), m_value(std::forward<Args>(args)...) {}

            result(failure f) noexcept
                : m_error(f.id()), m_has_value(false) {}

            ~result() noexcept;

            result(const result&);
            result& operator=(const result&);

            result(result&&) noexcept;
            result& operator=(result&&) noexcept;

            template <class U = std::remove_cv_t<T>> requires std::is_convertible_v<U, T>
            result& operator=(U&& v);

            // ###################################################
            // ---------------------------------------- OBSERVERS
            // #################################################

            JMAN_NO_DISCARD JMAN_FORCE_INLINE const T*              operator->() const noexcept { return jman::addressof(m_value); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T*                    operator->() noexcept { return jman::addressof(m_value); }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE const T&              operator*() const& noexcept { return m_value; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T&                    operator*() & noexcept { return m_value; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const T&&             operator*() const&& noexcept { return std::move(m_value); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE T&&                   operator*() && noexcept { return std::move(m_value); }
            
            JMAN_NO_DISCARD JMAN_FORCE_INLINE explicit operator     bool() const noexcept { return m_has_value; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool                  has_value() const noexcept { return m_has_value; }

            T&                                                      value() &;
            const T&                                                value() const&;
            T&&                                                     value() &&;
            const T&&                                               value() const&&;

            error_id                                                error() noexcept;

        private:
            union 
            {
                T m_value;
                error_id m_error;
            };

            bool m_has_value;
    }; // template <typename T> class result

    template<>
    class JMAN_NO_DISCARD result<void> {
    public:
        result() noexcept;
        result(failure) noexcept;

        JMAN_NO_DISCARD explicit operator bool() const noexcept;
        JMAN_NO_DISCARD bool has_value() const noexcept;
        JMAN_NO_DISCARD failure error() const noexcept;

        template<class E>
        JMAN_NO_DISCARD E get(failure id) noexcept;
    };

    template<class E>
    JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr failure new_error(E&& error) noexcept;
    template<class E>
    JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr failure new_error(E&& error, const context&) noexcept;
    template<class E>
    JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr failure new_error(E&& error, context&&) noexcept;

    template<class F, class... Handlers>
    auto handle(F&& operation, Handlers&&... handlers);

    template<class E>
    JMAN_NO_DISCARD E enum_code(failure id) noexcept;
}; // namespace jman