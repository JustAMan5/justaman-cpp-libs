#pragma once

#include <type_traits>

namespace jman 
{
    
    template <typename F, typename ...Args>
    struct is_callable 
    {
        static constexpr bool value = std::is_invocable_v<F, Args...>;
    };
    
    template <typename Ret, typename F, typename ...Args>
    struct is_callable_r 
    {
        static constexpr bool value = std::is_invocable_r_v<Ret, F, Args...>;
    };

    template<typename F, typename ...Args>
    inline constexpr bool is_callable_v = std::is_invocable_v<F, Args...>;

    template<typename Ret, typename F, typename ...Args>
    inline constexpr bool is_callable_r_v = std::is_invocable_v<Ret, F, Args...>;



    template <template <typename...> class T, typename U>
    struct is_specialization_of: std::false_type {};

    template <template <typename...> class T, typename ...Us>
    struct is_specialization_of<T, T<Us...>>: std::true_type {};

    template<template <typename...> class T, typename ...Us>
    inline constexpr bool is_specialization_of_v = is_specialization_of<T, Us...>();
}