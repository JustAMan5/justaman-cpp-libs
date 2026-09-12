#pragma once

#include <compare>
#include <cstddef>
#include <exception>
#include <new>
#include <justaman/meta/type_traits.hpp>
#include <justaman/memory/lifetime.hpp>
#include <justaman/utils/attributes.h>
#include "invoke.hpp"

namespace jman 
{
    template <class T> class lazy_value;

    namespace detail 
    {
        template<typename _Tp>
        inline constexpr bool is_lazy_value_v = false;
        template<typename _Tp>
        inline constexpr bool is_lazy_value_v<lazy_value<_Tp>> = true;    
    }

    class bad_lazy_access: public std::exception
    {
        public:
            bad_lazy_access() = default;
            virtual ~bad_lazy_access() = default;

#if __cpp_lib_constexpr_exceptions >= 202502L
            constexpr
#endif
            const char* what() const noexcept override
                { return "bad lazy access"; }
    };

    /**
        @brief:
            a wrapper class around an object that only gets created / initialized when accessing the element for the first time
    */
    template <class T>
    class lazy_value
    {
        static_assert(!std::is_reference_v<T>);
        static_assert(!std::is_void_v<T>);

        alignas(T) std::byte    m_buffer[sizeof(T)] {};
        bool                    m_initialized       {false};
    
        private:
            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr T*        m_ptr() noexcept { return std::launder(reinterpret_cast<T*>(m_buffer)); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr const T*  m_ptr() const noexcept { return std::launder(reinterpret_cast<const T*>(m_buffer)); }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr T&        m_value() noexcept { return *std::launder(reinterpret_cast<T*>(m_buffer)); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr const T&  m_value() const noexcept { return *std::launder(reinterpret_cast<const T*>(m_buffer)); }

        public:
            constexpr lazy_value() noexcept = default;

            constexpr lazy_value(const T&) noexcept(std::is_nothrow_copy_constructible_v<T>); // already constructed
            constexpr lazy_value(T&&) noexcept(std::is_nothrow_move_constructible_v<T>); // already constructed

            constexpr ~lazy_value() noexcept;

            constexpr lazy_value(const lazy_value& other) noexcept(std::is_nothrow_copy_constructible_v<T>);
            constexpr lazy_value& operator=(const lazy_value& other) noexcept(std::is_nothrow_copy_assignable_v<T>);

            constexpr lazy_value(lazy_value&& other) noexcept;
            constexpr lazy_value& operator=(lazy_value&& other) noexcept;
            
            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr bool                initialized() noexcept { return m_initialized; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE constexpr explicit operator   bool() noexcept { return m_initialized; }

            constexpr T&                            get() noexcept(std::is_nothrow_default_constructible_v<T>); // default constructs
            constexpr T&                            operator*() noexcept(std::is_nothrow_default_constructible_v<T>); // default constructs
            constexpr T*                            operator->() noexcept(std::is_nothrow_default_constructible_v<T>); // default constructs

#if __cpp_lib_constexpr_exceptions >= 202502L
            constexpr
#endif
            const T&                                get() const; 
#if __cpp_lib_constexpr_exceptions >= 202502L
            constexpr
#endif
            const T&                                operator*() const;
#if __cpp_lib_constexpr_exceptions >= 202502L
            constexpr
#endif
            const T*                                operator->() const;

            template <typename TT>
            requires std::is_convertible_v<TT, T>
            constexpr const T&                      value_or(TT&& _default) const noexcept;

            template <typename TT>
            requires std::is_convertible_v<TT, T>
            constexpr T&&                           value_or(TT&& _default) noexcept;

            constexpr void                          reset() noexcept;
            template <typename ...Args>
            constexpr void                          emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...> && std::is_nothrow_destructible_v<T>);

            template <typename F>
            constexpr auto                          and_then(F&& f) &;
            template <typename F>
            constexpr auto                          and_then(F&& f) const&;
            template <typename F>
            constexpr auto                          and_then(F&& f) &&;
            template <typename F>
            constexpr auto                          and_then(F&& f) const&&;

            template <typename F>
            constexpr auto                          or_else(F&& f) &&;
            template <typename F>
            constexpr auto                          or_else(F&& f) const&;

            template <typename T2>
            requires std::three_way_comparable_with<T, T2>
            constexpr auto                          operator<=>(const T2& other) noexcept;
    };
};


// ###############################################################################################
// --------------------------------- CONSTRUCTORS / DESTRUCTORS --------------------------------//
// ###############################################################################################

template <class T>
constexpr jman::lazy_value<T>::lazy_value(const T& x) noexcept(std::is_nothrow_copy_constructible_v<T>)
{
    jman::construct_at<T>(&m_buffer, x);
    m_initialized = true;
}
template <class T>
constexpr jman::lazy_value<T>::lazy_value(T&& x) noexcept(std::is_nothrow_move_constructible_v<T>)
{
    jman::construct_at<T>(&m_buffer, std::move(x));
    m_initialized = true;
}

template <class T>
constexpr jman::lazy_value<T>::~lazy_value() noexcept
{
    m_initialized = false;
    jman::destruct_at<T>(&m_buffer);
}

template <class T>
constexpr jman::lazy_value<T>::lazy_value(const lazy_value& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
{
    if constexpr (!std::is_trivially_copy_constructible_v<T>) {
        if (other.m_initialized == false) return;

        jman::construct_at<T>(&m_buffer, other.m_value());
        m_initialized = true;
    }
}
template <class T>
constexpr jman::lazy_value<T>::lazy_value& jman::lazy_value<T>::lazy_value::operator=(const lazy_value& other) noexcept(std::is_nothrow_copy_assignable_v<T>)
{
    if (this != &other)
    {
        if constexpr (!std::is_trivially_copy_constructible_v<T>) {
            if (other.m_initialized == false) return *this;

            jman::construct_at<T>(&m_buffer, other.m_value());
            m_initialized = true;
        }
    }

    return *this;
}

template <class T>
constexpr jman::lazy_value<T>::lazy_value(lazy_value&& other) noexcept
{
    if constexpr (!std::is_trivially_move_constructible_v<T>) {
        if (other.m_initialized == false) return;

        jman::construct_at<T>(&m_buffer, std::move(other.m_value()));
        m_initialized = true;
        other.m_initialized = false;
    }
}
template <class T>
constexpr jman::lazy_value<T>::lazy_value& jman::lazy_value<T>::lazy_value::operator=(lazy_value&& other) noexcept
{
    if (this != &other)
    {
        if constexpr (!std::is_trivially_copy_constructible_v<T>) {
            if (other.m_initialized == false) return *this;

            jman::construct_at<T>(&m_buffer, std::move(other.m_value()));
            m_initialized = true;
            other.m_initialized = false;
        }
    }

    return *this;
}

// ###############################################################################################
// ------------------------------------------ ACCESSORS  ---------------------------------------//
// ###############################################################################################

template <class T>
constexpr T& jman::lazy_value<T>::get() noexcept(std::is_nothrow_default_constructible_v<T>)
{
    if (!m_initialized)
        jman::construct_at<T>(&m_buffer);

    return m_value();
}
template <class T>
constexpr T& jman::lazy_value<T>::operator*() noexcept(std::is_nothrow_default_constructible_v<T>)
{
    return get();
}
template <class T>
constexpr T* jman::lazy_value<T>::operator->() noexcept(std::is_nothrow_default_constructible_v<T>)
{
    if (!m_initialized)
        jman::construct_at<T>(&m_buffer);

    return m_ptr();
}


template <class T>
#if __cpp_lib_constexpr_exceptions >= 202502L
constexpr
#endif
const T& jman::lazy_value<T>::get() const
{
    if (!m_initialized)
        throw bad_lazy_access();

    return m_value();
}
template <class T>
#if __cpp_lib_constexpr_exceptions >= 202502L
constexpr
#endif
const T& jman::lazy_value<T>::operator*() const
{
    if (!m_initialized)
        throw bad_lazy_access();

    return m_value();
}
template <class T>
#if __cpp_lib_constexpr_exceptions >= 202502L
constexpr
#endif
const T* jman::lazy_value<T>::operator->() const
{
    if (!m_initialized)
        throw bad_lazy_access();

    return m_ptr();
}

template <class T>
template <typename TT>
requires std::is_convertible_v<TT, T>
constexpr const T& jman::lazy_value<T>::value_or(TT&& _default) const noexcept
{
    return initialized() ? **this : static_cast<T>(std::forward<TT>(_default));
}

template <class T>
template <typename TT>
requires std::is_convertible_v<TT, T>
constexpr T&& jman::lazy_value<T>::value_or(TT&& _default) noexcept
{
    return initialized() ? std::move(**this) : static_cast<T>(std::forward<TT>(_default));
}


// ###############################################################################################
// ------------------------------------ CONDITIONAL BRANCHES -----------------------------------//
// ###############################################################################################

template <class T>
template <typename F>
constexpr auto jman::lazy_value<T>::and_then(F&& f) &
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(detail::is_lazy_value_v<R>,
        "The function passed to std::lazy_value<T>::and_then "
	    "must return a std::lazy_value"
    );

    if (initialized())
        return jman::invoke(std::forward<F>(f), m_value());
    else
        return R{};
}

template <class T>
template <typename F>
constexpr auto jman::lazy_value<T>::and_then(F&& f) const&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
    static_assert(detail::is_lazy_value_v<R>,
        "The function passed to std::lazy_value<T>::and_then "
	    "must return a std::lazy_value"
    );

    if (initialized())
        return jman::invoke(std::forward<F>(f), m_value());
    else
        return R{};
}

template <class T>
template <typename F>
constexpr auto jman::lazy_value<T>::and_then(F&& f) &&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, T>>;
    static_assert(detail::is_lazy_value_v<R>,
        "The function passed to std::lazy_value<T>::and_then "
	    "must return a std::lazy_value"
    );

    if (initialized())
        return jman::invoke(std::forward<F>(f), m_value());
    else
        return R{};
}

template <class T>
template <typename F>
constexpr auto jman::lazy_value<T>::and_then(F&& f) const&&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, const T>>;
    static_assert(detail::is_lazy_value_v<R>,
        "The function passed to std::lazy_value<T>::and_then "
	    "must return a std::lazy_value"
    );

    if (initialized())
        return jman::invoke(std::forward<F>(f), m_value());
    else
        return R{};
}

template <class T>
template <typename F>
constexpr auto jman::lazy_value<T>::or_else(F&& f) &&
{   
    using R = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(std::is_same_v<std::remove_cvref_t<R>, lazy_value>,
			"the function passed to std::lazy_value<T>::or_else "
			"must return a std::lazy_value<T>"
        );
    return initialized() ? *this : std::forward<F>(f)();
}
template <class T>
template <typename F>
constexpr auto jman::lazy_value<T>::or_else(F&& f) const&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(std::is_same_v<std::remove_cvref_t<R>, lazy_value>,
			"the function passed to std::lazy_value<T>::or_else "
			"must return a std::lazy_value<T>"
        );
    return initialized() ? std::move(*this) : std::forward<F>(f)();
}

// ###############################################################################################
// ----------------------------------------- MODIFIERS -----------------------------------------//
// ###############################################################################################

template <class T>
constexpr void jman::lazy_value<T>::reset() noexcept
{
    jman::destruct_at<T>(&m_buffer);
    m_initialized = false;
}
template <class T>
template <typename ...Args>
constexpr void jman::lazy_value<T>::emplace(Args&&... args) 
    noexcept(std::is_nothrow_constructible_v<T, Args...> && std::is_nothrow_destructible_v<T>)
{
    if (m_initialized)
        jman::destruct_at<T>(&m_buffer);
    
    m_initialized = false; // isnt it easier to just leave it normal?? we do this just in case constructor fails
    jman::construct_at<T>(&m_buffer, std::forward<Args>(args)...);
    m_initialized = true;
}