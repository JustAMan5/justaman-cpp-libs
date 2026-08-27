#ifndef JMAN_COMPILER_ATTRIBUTES_H_LIB
#define JMAN_COMPILER_ATTRIBUTES_H_LIB


// ##########################################################################################
// ------------------------------------- LIKELY & UNLIKELY ----------------------------------//
// ##########################################################################################

#if defined(__cplusplus) && __cplusplus >= 202002L // C++20 and beyond have them implemented

    #define JMAN_LIKELY(condition) (condition) [[likely]]
    #define JMAN_UNLIKELY(condition) (condition) [[unlikely]]

    // for global macros likes this people prefer to just use __builtin_except since doing it with the c++20 attributes wont work

#elif defined(__GNUC__) || defined(__clang__) // GCC and Clang have it as compiler extensions for both C and C++

    #define JMAN_LIKELY(condition) (__builtin_expect(!!(condition), 1))
    #define JMAN_UNLIKELY(condition) (__builtin_expect(!!(condition), 0))

#else // fallback to regular when not available

    #define JMAN_LIKELY(condition) (condition);
    #define JMAN_UNLIKELY(condition) (condition);

#endif

// #######################################################################################
// ------------------------------------- FORCE INLINE ----------------------------------//
// #######################################################################################

#if defined(_MSC_VER)
    #define JMAN_FORCE_INLINE inline __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define JMAN_FORCE_INLINE inline __attribute__((always_inline))
#else
    #define JMAN_FORCE_INLINE inline
#endif

// #####################################################################################
// ------------------------------------- NO DISCARD ----------------------------------//
// #####################################################################################

#if (defined(__cplusplus) && __cplusplus >= 201703L) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L) 
    // C++17 & C23 and beyond have them implemented natively

    #define JMAN_NO_DISCARD [[nodiscard]]
    #define JMAN_NO_DISCARD_REASON(x) [[nodiscard(x)]]

#elif defined(__GNUC__) || defined(__clang__) // GCC and Clang have it as compiler extensions for both C and C++

    #define JMAN_NO_DISCARD __attribute__((warn_unused_result))
    #define JMAN_NO_DISCARD_REASON(x) __attribute__((warn_unused_result))

#elif defined(_MSC_VER)

    #define JMAN_NO_DISCARD _Check_return_
    #define JMAN_NO_DISCARD_REASON(x) _Check_return_

#else // fallback to just empty 

     #define JMAN_NO_DISCARD
     #define JMAN_NO_DISCARD_REASON

#endif

// #####################################################################################
// ------------------------------------- DEPRECATED ----------------------------------//
// #####################################################################################

#if (defined(__cplusplus) && __cplusplus >= 201402L) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L) 
    // C++14 & C23 and beyond have them implemented natively

    #define JMAN_DEPRECATED [[deprecated]]
    #define JMAN_DEPRECATED_SUGGEST(x) [[deprecated("Use [" x "] instead.")]]

#elif defined(__GNUC__) || defined(__clang__) // GCC and Clang have it as compiler extensions for both C and C++

    #define JMAN_DEPRECATED __attribute__((deprecated))
    #define JMAN_DEPRECATED_SUGGEST(x) __attribute__((deprecated("Use [" x "] instead.")))

#elif defined(_MSC_VER)

    #define JMAN_DEPRECATED __declspec(deprecated)
    #define JMAN_DEPRECATED_SUGGEST(x) __declspec(deprecated("Use [" x "] instead."))

#else // fallback to just empty 

     #define JMAN_DEPRECATED
     #define JMAN_DEPRECATED_SUGGEST(x)

#endif

#endif // JMAN_COMPILER_ATTRIBUTES_H_LIB