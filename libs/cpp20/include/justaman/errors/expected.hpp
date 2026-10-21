#pragma once

#include "justaman/memory/lifetime.hpp"
#include "justaman/meta/invoke.hpp"
#include <justaman/utils/attributes.h>
#include "safe_exceptions.hpp"
#include <exception>
#include <type_traits>
#include <utility>

/**
    @todo
        add void specialization and check for it to work with references and special types with const and such
        and also mark methods as noexcept
*/

namespace jman 
{
    template <class E>
    class unexpected 
    {
        public:
            unexpected(const unexpected&) = default;
            unexpected( unexpected&& ) = default;

            template <class Err = E> requires std::is_convertible_v<Err, E>
            explicit unexpected(Err&& e): m_value(std::move(e)) {}
            template <class... Args>
            unexpected(std::in_place_t, Args&&... args): m_value(std::forward<Args>(args)...) {}

            JMAN_NO_DISCARD JMAN_FORCE_INLINE const E&        error() const& noexcept { return m_value; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE E&              error() & noexcept { return m_value; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const E&&       error() const&& noexcept { return std::move(m_value); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE E&&             error() && noexcept { return std::move(m_value); }

        private:
            E m_value;
    };

    // tells the compiler that if the user writes unexcpected(E), they surely meant unexpected<E>{}
    template<class E>
    unexpected(E) -> unexpected<E>;

    struct unexpect_t
    {
        explicit unexpect_t() = default;
    };
    inline constexpr unexpect_t unexpect{};

    template <class E>
    class bad_expected_access: std::exception {
        constexpr const char* what() const noexcept override { return "jman::bad_expected_access()"; }
    };
}

namespace jman 
{
    template <class T, class E>
    class JMAN_DEPRECATED expected 
    {
        static_assert(!std::is_reference_v<T>);
        static_assert(!std::is_function_v<T>);
        static_assert(!std::is_same_v<std::remove_cv_t<T>, std::in_place_t>);
        static_assert(!std::is_same_v<std::remove_cv_t<T>, unexpect_t>);
        static_assert(
            std::is_object_v<E> && !std::is_array_v<E> && 
            !std::is_const_v<E> && !std::is_volatile_v<E>
        );

        public:
            using value_type = T;
            using error_type = E;
            using unexpected_type = unexpected<E>;

            // ###################################################
            // ------------------------------------- CONSTRUCTORS
            // #################################################
            expected(): m_value(), m_has_value(true) {}

            template<class U, class G> requires std::is_convertible_v<U, T> && std::is_convertible_v<G, E>
            expected(const expected<U, G>& other);
            template<class U, class G> requires std::is_convertible_v<U, T> && std::is_convertible_v<G, E>
            expected(expected<U, G>&& other);

            template<class U = std::remove_cv_t<T>> requires std::is_convertible_v<U, T>
            expected(U&& v) : m_has_value(true), m_value(std::move(v)) {}

            template <class G> requires std::is_convertible_v<const G&, E>
            expected(const unexpected<G>& e): m_has_value(false), m_unex(e.error()) {}
            template <class G> requires std::is_convertible_v<G, E>
            expected(unexpected<G>&& e): m_has_value(false), m_unex(std::move(e.error())) {}

            template <class... Args> requires std::is_constructible_v<T, Args...>
            expected( std::in_place_t, Args&&... args ): m_has_value(true), m_value(std::forward<Args>(args)...) {}
            template <class... Args> requires std::is_constructible_v<E, Args...>
            expected( jman::unexpect_t, Args&&... args ): m_has_value(false), m_unex(std::forward<Args>(args)...) {}

            ~expected() noexcept;

            expected(const expected&);
            expected& operator=(const expected&);

            expected(expected&&) noexcept;
            expected& operator=(expected&&) noexcept;

            template <class U = std::remove_cv_t<T>> requires std::is_convertible_v<U, T>
            expected& operator=(U&& v);

            template <class G> requires std::is_convertible_v<G, E>
            expected& operator=( const unexpected<G>& other );
            template <class G> requires std::is_convertible_v<G, E>
            expected& operator=(unexpected<G>&& other);

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
            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool                  has_value() const noexcept { return m_has_value;}
            

            T&                                                      value() &;
            const T&                                                value() const&;
            T&&                                                     value() &&;
            const T&&                                               value() const&&;

            const E&                                                error() const& noexcept;
            E&                                                      error() & noexcept;
            const E&&                                               error() const&& noexcept;
            E&&                                                     error() && noexcept;

            template <class U = std::remove_cv_t<T>> requires std::is_convertible_v<U, T>
            T                                                       value_or(U&& default_value) const&;

            template <class U = std::remove_cv_t<T>> requires std::is_convertible_v<U, T>
            T                                                       value_or(U&& default_value) &&;

            template <class G = E> requires std::is_convertible_v<G, E>
            E                                                       error_or(G&& default_value) const&;

            template <class G = E> requires std::is_convertible_v<G, E>
            E                                                       error_or(G&& default_value) &&;

            // ###################################################
            // --------------------------------------- OPERATIONS
            // #################################################

            template <class F>
            auto and_then(F&& f) &;

            template <class F>
            auto and_then(F&& f) const&;

            template <class F>
            auto and_then(F&& f) &&;

            template <class F>
            auto and_then(F&& f) const&&;


            template <class F>
            auto or_else(F&& f) &;

            template <class F>
            auto or_else(F&& f) const&;

            template <class F>
            auto or_else(F&& f) &&;

            template <class F>
            auto or_else(F&& f) const&&;
            
        private:
            union {
                T m_value;
                E m_unex;
            };
            bool m_has_value;

        private:
            template <class U>
            void m_assign_value(U&&);
            template <class G>
            void m_assign_unex(G&&);
    };

    template <class E>
    class expected<void, E>
    {
        static_assert(
            std::is_object_v<E> && !std::is_array_v<E> && 
            !std::is_const_v<E> && !std::is_volatile_v<E>
        );

        public:
            using value_type = void;
            using error_type = E;
            using unexpected_type = unexpected<E>;

            // ###################################################
            // ------------------------------------- CONSTRUCTORS
            // #################################################
            expected(): m_has_value(true) {}

            template <class G> requires std::is_convertible_v<const G&, E>
            expected(const unexpected<G>& e);
            template <class G> requires std::is_convertible_v<G, E>
            expected(unexpected<G>&& e);

            template <class... Args> requires std::is_constructible_v<E, Args...>
            expected( jman::unexpect_t, Args&&... args );

            ~expected() noexcept;

            expected(const expected&);
            expected& operator=(const expected&);

            expected(expected&&) noexcept;
            expected& operator=(expected&&) noexcept;

            template <class G> requires std::is_convertible_v<G, E>
            expected& operator=( const unexpected<G>& other );
            template <class G> requires std::is_convertible_v<G, E>
            expected& operator=(unexpected<G>&& other);

            // ###################################################
            // ---------------------------------------- OBSERVERS
            // #################################################

            JMAN_FORCE_INLINE void                                  operator*() const noexcept      { return void(); }
            
            JMAN_NO_DISCARD JMAN_FORCE_INLINE explicit operator     bool() const noexcept           { return m_has_value; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool                  has_value() const noexcept      { return m_has_value;}
            

            JMAN_FORCE_INLINE void                                  value() const& noexcept         { return void(); }
            JMAN_FORCE_INLINE void                                  value() && noexcept             { return void(); }

            JMAN_FORCE_INLINE const E&                              error() const& noexcept; 
            JMAN_FORCE_INLINE E&                                    error() & noexcept;
            JMAN_FORCE_INLINE const E&&                             error() const&& noexcept;
            JMAN_FORCE_INLINE E&&                                   error() && noexcept;

            template <class G = E> requires std::is_convertible_v<G, E>
            E                                                       error_or(G&& default_value) const&;

            template <class G = E> requires std::is_convertible_v<G, E>
            E                                                       error_or(G&& default_value) &&;

            // ###################################################
            // --------------------------------------- OPERATIONS
            // #################################################

            template <class F>
            auto                                                    and_then(F&& f) &;

            template <class F>
            auto                                                    and_then(F&& f) const&;

            template <class F>
            auto                                                    and_then(F&& f) &&;

            template <class F>
            auto                                                    and_then(F&& f) const&&;


            template <class F>
            auto                                                    or_else(F&& f) &;

            template <class F>
            auto                                                    or_else(F&& f) const&;

            template <class F>
            auto                                                    or_else(F&& f) &&;

            template <class F>
            auto                                                    or_else(F&& f) const&&;
            
        private:
            alignas(E) std::byte        m_buffer[sizeof(E)];
            bool                        m_has_value;

        private:
            template <class G>
            void                        m_assign_unex(G&&);
    };
} // namespace jman


namespace jman::detail 
{
    template<typename T>
    inline constexpr bool is_expected = false;
    template<typename T, typename E>
    inline constexpr bool is_expected<jman::expected<T, E>> = true;
} // namespace jman::detail

namespace jman
{
// ###############################################################################################
// --------------------------------- REGULAR - PRIVATE METHODS  --------------------------------//
// ###############################################################################################

template <class T, class E>
template <class U>
void expected<T, E>::m_assign_value(U&& x)
{
    if (m_has_value)
        m_value = std::forward<U>(x);
    else 
    {
        jman::destruct_at(jman::addressof(m_unex));
        jman::construct_at(jman::addressof(m_value), std::forward<U>(x));
	    m_has_value = true;
	}
}

template <class T, class E>
template <class G>
void expected<T, E>::m_assign_unex(G&& x)
{
    if (m_has_value) {
        jman::destruct_at(jman::addressof(m_value));
        jman::construct_at(jman::addressof(m_unex), std::forward<G>(x));
        m_has_value = false;
    }
    else 
        m_unex = std::forward<G>(x);
}


// ###############################################################################################
// ------------------------------- REGULAR - CONSTRUCTORS / DESTRUCTORS ------------------------//
// ###############################################################################################

template <class T, class E>
template<class U, class G> requires std::is_convertible_v<U, T> && std::is_convertible_v<G, E>
expected<T, E>::expected(const expected<U, G>& other)
    : m_has_value(other.m_has_value)
{
    if (m_has_value)
        jman::construct_at(jman::addressof(m_value), other.m_value);
    else
	    jman::construct_at(jman::addressof(m_unex), other.m_unex);
}

template <class T, class E>
template<class U, class G> requires std::is_convertible_v<U, T> && std::is_convertible_v<G, E>
expected<T, E>::expected(expected<U, G>&& other)
    : m_has_value(other.m_has_value)
{
    if (m_has_value)
        jman::construct_at(jman::addressof(m_value), std::move(other).m_value);
    else
	    jman::construct_at(jman::addressof(m_unex), std::move(other).m_unex);
}

template <class T, class E>
expected<T, E>::~expected() noexcept
{
    if (m_has_value)
        jman::destruct_at(jman::addressof(m_value));
    else
	    jman::destruct_at(jman::addressof(m_unex));
}

template <class T, class E>
expected<T, E>::expected(const expected& other)
    : m_has_value(other.m_has_value)
{
    if (m_has_value)
        jman::construct_at(jman::addressof(m_value), other.m_value);
    else
	    jman::construct_at(jman::addressof(m_unex), other.m_unex);
}
template <class T, class E>
expected<T, E>& expected<T, E>::operator=(const expected& other)
{
    if (this != &other) return *this;

    if (other.m_has_value)
        m_assign_value(other.m_value);
    else
        m_assign_unex(other.m_unex);

    return *this;
}

template <class T, class E>
expected<T, E>::expected(expected&& other) noexcept
    : m_has_value(other.m_has_value)
{
    if (m_has_value)
        jman::construct_at(jman::addressof(m_value), std::move(other).m_value);
    else
	    jman::construct_at(jman::addressof(m_unex), std::move(other).m_unex);
}
template <class T, class E>
expected<T, E>& expected<T, E>::operator=(expected&& other) noexcept
{
    if (this != &other) return *this;

    if (other.m_has_value)
        m_assign_value(std::move(other.m_value));
    else
        m_assign_unex(std::move(other.m_unex));

    return *this;
}

template <class T, class E>
template <class U> requires std::is_convertible_v<U, T>
expected<T, E>& expected<T, E>::operator=(U&& v)
{
    m_assign_value(std::forward<U>(v));
    return *this;
}

template <class T, class E>
template <class G> requires std::is_convertible_v<G, E>
expected<T, E>& expected<T, E>::operator=( const unexpected<G>& other )
{
    m_assign_unex(other.error());
    return *this;
}
template <class T, class E>
template <class G> requires std::is_convertible_v<G, E>
expected<T, E>& expected<T, E>::operator=(unexpected<G>&& other)
{
    m_assign_unex(std::move(other.error()));
    return *this;
}

// ###############################################################################################
// ------------------------------------- REGULAR -  OBSERVERS ----------------------------------//
// ###############################################################################################

template <class T, class E>
T& expected<T, E>::value() &
{
    if (!m_has_value) JMAN_THROW(bad_expected_access<E>{});
    return m_value;
}
template <class T, class E>
const T& expected<T, E>::value() const&
{
    if (!m_has_value) JMAN_THROW(bad_expected_access<E>{});
    return m_value;
}
template <class T, class E>
T&& expected<T, E>::value() &&
{
    if (!m_has_value) JMAN_THROW(bad_expected_access<E>{});
    return std::move(m_value);
}
template <class T, class E>
const T&& expected<T, E>::value() const&&
{
    if (!m_has_value) JMAN_THROW(bad_expected_access<E>{});
    return std::move(m_value);
}



template <class T, class E>
const E& expected<T, E>::error() const& noexcept
{
    return m_unex;
}
template <class T, class E>
E& expected<T, E>::error() & noexcept
{
    return m_unex;
}
template <class T, class E>
const E&& expected<T, E>::error() const&& noexcept
{
    return std::move(m_unex);
}
template <class T, class E>
E&& expected<T, E>::error() && noexcept
{
    return std::move(m_unex);
}


template <class T, class E>
template <class U> requires std::is_convertible_v<U, T>
T expected<T, E>::value_or(U&& default_value) const&
{
    if (!m_has_value) return static_cast<T>(std::forward<U>(default_value));
    return m_value;
}

template <class T, class E>
template <class U> requires std::is_convertible_v<U, T>
T expected<T, E>::value_or(U&& default_value) &&
{
    if (!m_has_value) return static_cast<T>(std::forward<U>(default_value));
    return std::move(m_value);
}


template <class T, class E>
template <class G> requires std::is_convertible_v<G, E>
E expected<T, E>::error_or(G&& default_value) const&
{
    if (m_has_value) return static_cast<E>(std::forward<G>(default_value));
    return m_unex;
}

template <class T, class E>
template <class G> requires std::is_convertible_v<G, E>
E expected<T, E>::error_or(G&& default_value) &&
{
    if (m_has_value) return static_cast<E>(std::forward<G>(default_value));
    return std::move(m_unex);
}



template <class T, class E>
template <class F>
auto expected<T, E>::and_then(F&& f) &
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<T, E>::and_then "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::error_type, E>,
        "the function passed to jman::expected<T, E>::and_then "
        "must return a jman::expected with the same error_type"
    );
    if (m_has_value)
        return jman::invoke(std::forward<F>(f), m_value);
    else
        return R(unexpect, m_unex);
}

template <class T, class E>
template <class F>
auto expected<T, E>::and_then(F&& f) const&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<T, E>::and_then "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::error_type, E>,
        "the function passed to jman::expected<T, E>::and_then "
        "must return a jman::expected with the same error_type"
    );
    if (m_has_value)
        return jman::invoke(std::forward<F>(f), m_value);
    else
        return R(unexpect, m_unex);
}

template <class T, class E>
template <class F>
auto expected<T, E>::and_then(F&& f) &&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, T&&>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<T, E>::and_then "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::error_type, E>,
        "the function passed to jman::expected<T, E>::and_then "
        "must return a jman::expected with the same error_type"
    );
    if (m_has_value)
        return jman::invoke(std::forward<F>(f), std::move(m_value));
    else
        return R(unexpect, std::move(m_unex));
}

template <class T, class E>
template <class F>
auto expected<T, E>::and_then(F&& f) const&&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, const T&&>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<T, E>::and_then "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::error_type, E>,
        "the function passed to jman::expected<T, E>::and_then "
        "must return a jman::expected with the same error_type"
    );
    if (m_has_value)
        return jman::invoke(std::forward<F>(f), std::move(m_value));
    else
        return R(unexpect, std::move(m_unex));
}

template <class T, class E>
template <class F>
auto expected<T, E>::or_else(F&& f) &
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<T, E>::or_else "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::value_type, E>,
        "the function passed to jman::expected<T, E>::or_else "
        "must return a jman::expected with the same value_type"
    );
    if (m_has_value)
        return R(std::in_place, m_value);
    else
        return jman::invoke(std::forward<F>(f), m_unex);
}

template <class T, class E>
template <class F>
auto expected<T, E>::or_else(F&& f) const&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, const E&>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<T, E>::or_else "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::value_type, E>,
        "the function passed to jman::expected<T, E>::or_else "
        "must return a jman::expected with the same value_type"
    );
    if (m_has_value)
        return R(std::in_place, m_value);
    else
        return jman::invoke(std::forward<F>(f), m_unex);
}

template <class T, class E>
template <class F>
auto expected<T, E>::or_else(F&& f) &&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, E&&>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<T, E>::or_else "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::value_type, E>,
        "the function passed to jman::expected<T, E>::or_else "
        "must return a jman::expected with the same value_type"
    );
    if (m_has_value)
        return R(std::in_place, std::move(m_value));
    else
        return jman::invoke(std::forward<F>(f), std::move(m_unex));
}

template <class T, class E>
template <class F>
auto expected<T, E>::or_else(F&& f) const&&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, const E&&>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<T, E>::or_else "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::value_type, E>,
        "the function passed to jman::expected<T, E>::or_else "
        "must return a jman::expected with the same value_type"
    );
    if (m_has_value)
        return R(std::in_place, std::move(m_value));
    else
        return jman::invoke(std::forward<F>(f), std::move(m_unex));
}



// ###############################################################################################
// -------------------------------------- VOID - PRIVATE METHODS  ----------------.....----------------//
// ###############################################################################################

template <class E>
template <class G>
void expected<void, E>::m_assign_unex(G&& x)
{
    if (!m_has_value)
        jman::destruct_at<E>(jman::addressof(error()));
    jman::construct_at<E>(jman::addressof(error()), static_cast<E>(std::forward<G>(x)));
    m_has_value = false;
}


// ###############################################################################################
// ------------------------------- VOID -  CONSTRUCTORS / DESTRUCTORS --------------------------//
// ###############################################################################################

template <class E>
template <class G> requires std::is_convertible_v<const G&, E>
expected<void, E>::expected(const unexpected<G>& e)
    : m_has_value(false)
{
    m_assign_unex(e.error());
}

template <class E>
template <class G> requires std::is_convertible_v<G, E>
expected<void, E>::expected(unexpected<G>&& e)
    : m_has_value(false)
{
    m_assign_unex(std::move(e.error()));
}

template <class E>
template <class... Args> requires std::is_constructible_v<E, Args...>
expected<void, E>::expected( jman::unexpect_t, Args&&... args )
    : m_has_value(false)
{
    m_assign_unex(E{std::forward<Args>(args)...});
}

template <class E>
expected<void, E>::~expected() noexcept
{
    if (!m_has_value)
	    jman::destruct_at<E>(jman::addressof(error()));
}

template <class E>
expected<void, E>::expected(const expected& other)
    : m_has_value(other.m_has_value)
{
    if (!m_has_value)
	    jman::construct_at(jman::addressof(error()), other.error());
}
template <class E>
expected<void, E>& expected<void, E>::operator=(const expected& other)
{
    if (this != &other) return *this;

    if (!other.m_has_value)
        m_assign_unex(other.error());

    return *this;
}

template <class E>
expected<void, E>::expected(expected&& other) noexcept
    : m_has_value(other.m_has_value)
{
    if (!m_has_value)
	    jman::construct_at(jman::addressof(error()), std::move(other.error()));
}
template <class E>
expected<void, E>& expected<void, E>::operator=(expected&& other) noexcept
{
    if (this != &other) return *this;

    if (!other.m_has_value)
        m_assign_unex(std::move(other.error()));

    return *this;
}

template <class E>
template <class G> requires std::is_convertible_v<G, E>
expected<void, E>& expected<void, E>::operator=( const unexpected<G>& other )
{
    m_assign_unex(other.error());
    return *this;
}
template <class E>
template <class G> requires std::is_convertible_v<G, E>
expected<void, E>& expected<void, E>::operator=(unexpected<G>&& other)
{
    m_assign_unex(std::move(other.error()));
    return *this;
}

// ###############################################################################################
// ----------------------------------------- OBSERVERS -----------------------------------------//
// ###############################################################################################

template <class E>
JMAN_FORCE_INLINE const E& expected<void, E>::error() const& noexcept
{
    return *std::launder(reinterpret_cast<E*>(&m_buffer));
}
template <class E>
JMAN_FORCE_INLINE E& expected<void, E>::error() & noexcept
{
    return *std::launder(reinterpret_cast<E*>(&m_buffer));
}
template <class E>
JMAN_FORCE_INLINE const E&& expected<void, E>::error() const&& noexcept
{
    return std::move(*std::launder(reinterpret_cast<E*>(&m_buffer)));
}
template <class E>
JMAN_FORCE_INLINE E&& expected<void, E>::error() && noexcept
{
    return std::move(*std::launder(reinterpret_cast<E*>(&m_buffer)));
}

template <class E>
template <class G> requires std::is_convertible_v<G, E>
E expected<void, E>::error_or(G&& default_value) const&
{
    if (m_has_value) return static_cast<E>(std::forward<G>(default_value));
    return error();
}

template <class E>
template <class G> requires std::is_convertible_v<G, E>
E expected<void, E>::error_or(G&& default_value) &&
{
    if (m_has_value) return static_cast<E>(std::forward<G>(default_value));
    return std::move(error());
}

// ###############################################################################################
// ----------------------------------------- OBSERVERS -----------------------------------------//
// ###############################################################################################

template <class E>
template <class F>
auto expected<void, E>::and_then(F&& f) &
{
    using R = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<void, E>::and_then "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::error_type, E>,
        "the function passed to jman::expected<void, E>::and_then "
        "must return a jman::expected with the same error_type"
    );
    if (m_has_value)
        return jman::invoke(std::forward<F>(f));
    else
        return R(unexpect, error());
}

template <class E>
template <class F>
auto expected<void, E>::and_then(F&& f) const&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<void, E>::and_then "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::error_type, E>,
        "the function passed to jman::expected<void, E>::and_then "
        "must return a jman::expected with the same error_type"
    );
    if (m_has_value)
        return jman::invoke(std::forward<F>(f));
    else
        return R(unexpect, error());
}

template <class E>
template <class F>
auto expected<void, E>::and_then(F&& f) &&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<void, E>::and_then "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::error_type, E>,
        "the function passed to jman::expected<void, E>::and_then "
        "must return a jman::expected with the same error_type"
    );
    if (m_has_value)
        return jman::invoke(std::forward<F>(f));
    else
        return R(unexpect, std::move(error()));
}

template <class E>
template <class F>
auto expected<void, E>::and_then(F&& f) const&&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<void, E>::and_then "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::error_type, E>,
        "the function passed to jman::expected<void, E>::and_then "
        "must return a jman::expected with the same error_type"
    );
    if (m_has_value)
        return jman::invoke(std::forward<F>(f));
    else
        return R(unexpect, std::move(error()));
}

template <class E>
template <class F>
auto expected<void, E>::or_else(F&& f) &
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<void, E>::or_else "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::value_type, E>,
        "the function passed to jman::expected<void, E>::or_else "
        "must return a jman::expected with the same value_type"
    );
    if (m_has_value)
        return R();
    else
        return jman::invoke(std::forward<F>(f), error());
}

template <class E>
template <class F>
auto expected<void, E>::or_else(F&& f) const&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, const E&>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<void, E>::or_else "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::value_type, E>,
        "the function passed to jman::expected<void, E>::or_else "
        "must return a jman::expected with the same value_type"
    );
    if (m_has_value)
        return R();
    else
        return jman::invoke(std::forward<F>(f), error());
}

template <class E>
template <class F>
auto expected<void, E>::or_else(F&& f) &&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, E&&>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<void, E>::or_else "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::value_type, E>,
        "the function passed to jman::expected<void, E>::or_else "
        "must return a jman::expected with the same value_type"
    );
    if (m_has_value)
        return R();
    else
        return jman::invoke(std::forward<F>(f), std::move(error()));
}

template <class E>
template <class F>
auto expected<void, E>::or_else(F&& f) const&&
{
    using R = std::remove_cvref_t<std::invoke_result_t<F, const E&&>>;
    static_assert(detail::is_expected<E>,
        "the function passed to jman::expected<void, E>::or_else "
        "must return a jman::expected"
    );
    static_assert(
        std::is_same_v<typename R::value_type, E>,
        "the function passed to jman::expected<void, E>::or_else "
        "must return a jman::expected with the same value_type"
    );
    if (m_has_value)
        return R();
    else
        return jman::invoke(std::forward<F>(f), std::move(error()));
}
} // namespace jman