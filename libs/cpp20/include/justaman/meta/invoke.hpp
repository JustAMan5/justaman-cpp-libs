#pragma once

#include <utility>

namespace jman 
{   
    namespace detail 
    {
        // pointers to member methods of a class, first argument is the function adress itself, the second arg is for the object that refers to
        template <typename Base, typename T, typename Derived, typename... Args>
        auto invoke_impl(T Base::*pmf, Derived&& obj, Args&&... args)
            -> decltype((std::forward<Derived>(obj).*pmf)(std::forward<Args>(args)...)) 
        {
            return (std::forward<Derived>(obj).*pmf)(std::forward<Args>(args)...);
        }

        // invocable object like lambdas functions etc etc
        template <typename F, typename... Args>
        auto invoke_impl(F&& f, Args&&... args)
            -> decltype(std::forward<F>(f)(std::forward<Args>(args)...)) 
        {
            return std::forward<F>(f)(std::forward<Args>(args)...);
        }
    } // namespace jman::detail

    /**
        @brief
            same functionallity as regular std::invoke that invokes any type of callable object
    */
    template <typename F, typename... Args>
    auto invoke(F&& f, Args&&... args) 
        -> decltype(detail::invoke_impl(std::forward<F>(f), std::forward<Args>(args)...)) 
    {
        return detail::invoke_impl(std::forward<F>(f), std::forward<Args>(args)...);
    }
} // namespace jman