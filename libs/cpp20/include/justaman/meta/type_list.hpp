#pragma once

#include <cstddef>
#include "type_traits.hpp"

namespace jman 
{
    template <typename... T>
    struct type_list {};


    

    template <typename TList>
    struct size;

    template <typename ...T>
    struct size<type_list<T...>>
    {
        static constexpr std::size_t value = sizeof...(T);
    };

    template <typename TList>
    inline constexpr std::size_t size_v = size<TList>::value;





    template <std::size_t Index, typename TList>
    struct at;

    template <std::size_t Index, typename Head, typename... Tail>
    struct at<Index, type_list<Head, Tail...>> 
    {
        using type = typename at<Index - 1, type_list<Tail...>>::type; 
    };

    template <typename Head, typename... Tail>
    struct at<0, type_list<Head, Tail...>> 
    {
        using type = Head;
    };

    template <std::size_t Index, typename TList>
    using at_t = typename at<Index, TList>::type;





    template <typename T, typename TList>
    struct contains;

    template <typename T>
    struct contains<T, type_list<>> 
    {
        static constexpr bool value = false;
    };

    template <typename T, typename ...Ts>
    struct contains<T, type_list<Ts...>> 
    {
        static constexpr bool value = std::disjunction<std::is_same<T, Ts>...>::value;
    };

    template <typename T, typename TList>
    inline constexpr bool contains_v = contains<T, TList>::value;



    template <typename T, typename TList>
    struct index;

    template <typename T, typename ...Ts>
    struct index<T, type_list<T, Ts...>>
    {
        static constexpr std::size_t value = 0;
    };

    template <typename T1, typename T2, typename ...Ts>
    struct index<T1, type_list<T2, Ts...>>
    {
        static constexpr std::size_t value = 1 + index<type_list<Ts...>, T1>::value;
    };

    template <typename T, typename TList>
    inline constexpr bool index_v = index<T, TList>::value;





    template <typename TList>
    struct front;

    template <typename Head, typename ...Tail>
    struct front<type_list<Head, Tail...>> {
        using type = Head;
    };

    template <typename TList>
    using front_t = typename front<TList>::type;




    template <typename TList>
    struct back;

    template <typename T>
    struct back<type_list<T>> 
    {
        using type = T;
    };

    template <typename Head, typename ...Tail>
    struct back<type_list<Head, Tail...>> 
    {
        using type = typename back<type_list<Tail...>>::type;
    };

    template <typename TList>
    using back_t = typename back<TList>::type;
    


    template <typename T, typename TList>
    struct append;

    template <typename V, typename ...Ts>
    struct append<V, type_list<Ts...>> 
    {
        using type = type_list<Ts..., V>;
    };

    template <typename T, typename TList>
    using append_t = typename append<T, TList>::type;



    template <typename TList, typename T>
    struct prepend;

    template <typename T, typename ...Ts>
    struct prepend<type_list<Ts...>, T> {
        using type = type_list<T, Ts...>;
    };

    template <typename T, typename TList>
    using prepend_t = typename prepend<T, TList>::type;


    template <typename TList>
    struct for_each;

    template <typename ...Ts>
    struct for_each<type_list<Ts...>>
    {
        template <typename Func>
        requires (jman::is_callable_v<Func, Ts>, ...)
        void operator()(Func&& func) const
        {
            (func(Ts{}), ...);
        }
    };

    template <typename TList>
    inline constexpr for_each<TList> for_each_v = for_each<TList>{};
} // namespace jman