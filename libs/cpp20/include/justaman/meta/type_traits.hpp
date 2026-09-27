#pragma once

#include <type_traits>

namespace jman 
{   
    /**
        @brief 
            utility for knowing if for exemple std::vector<T> is a specialization of std::vector (true)
            otherwise checking if std::vector<T> is a specialization of std::map returns false
    */
    ///@{
    // the template <typename...> thing tells the compilers class T is a templated class that accepts a variadic types
    template <template <typename...> class T, typename U>
    struct is_specialization_of: std::false_type {};

    template <template <typename...> class T, typename ...Us>
    struct is_specialization_of<T, T<Us...>>: std::true_type {};

    template<template <typename...> class T, typename ...Us>
    inline constexpr bool is_specialization_of_v = is_specialization_of<T, Us...>();
    ///@}

    /**
        @brief 
            helper trait to extract the underlying type of a raw pointer or smart pointer
    */
    ///@{
    template <typename T, typename = void>
    struct pointer_element_type {
        using type = T; // fallback if it's already a regular object
    };

    // specialization for raw pointers
    template <typename T>
    struct pointer_element_type<T*> {
        using type = T;
    };

    // specialization for smart pointers / wrappers that define element_type
    // if the expression inside the std::void_t is not valid, discards it and uses any fallback instead
    template <typename T>
    struct pointer_element_type<T, std::void_t<typename T::element_type>> {
        using type = typename T::element_type;
    };

    template <typename T>
    using pointer_element_type_t = typename pointer_element_type<std::remove_cvref_t<T>>::type;
    ///@}

} // namespace jman