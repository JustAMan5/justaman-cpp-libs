#pragma once

#include <type_traits>

namespace jman 
{
    template <template <typename...> class T, typename U>
    struct is_specialization_of: std::false_type {};
    template <template <typename...> class T, typename ...Us>
    struct is_specialization_of<T, T<Us...>>: std::true_type {};

    template<template <typename...> class T, typename ...Us>
    inline constexpr bool is_specialization_of_v = is_specialization_of<T, Us...>();
} // namespace jman