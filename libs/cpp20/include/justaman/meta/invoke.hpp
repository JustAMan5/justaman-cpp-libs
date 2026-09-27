#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace jman 
{   
    namespace detail 
    {
        // utility concept to check if object has the .get() method
        template <typename T>
        concept invoke_has_get_method = requires(T&& obj) { { obj.get() }; };

        // utility for checking if a member class func is nothrow
        template <typename Base, typename Signature, typename Derived, typename... Args>
        struct is_nothrow_memptr_invocable {
            static consteval bool get_value() noexcept {
                using object_t = std::decay_t<Derived>;

                constexpr bool is_member_function = std::is_function_v<Signature>;
                constexpr bool is_wrapped = invoke_has_get_method<Derived>;
                constexpr bool is_derived_object = std::derived_from<object_t, Base>;
                
                if constexpr (is_member_function) {
                    if constexpr (is_derived_object)
                        return noexcept((std::declval<Derived>().*std::declval<Signature Base::*>()) (std::declval<Args>()...));
                    else if constexpr (is_wrapped)
                        return noexcept((std::declval<Derived>().get().*std::declval<Signature Base::*>()) (std::declval<Args>()...));
                    else
                        return noexcept(((*std::declval<Derived>()).*std::declval<Signature Base::*>()) (std::declval<Args>()...));
                } else {
                    if constexpr (is_derived_object)
                        return noexcept(std::declval<Derived>().*std::declval<Signature Base::*>());
                    else if constexpr (is_wrapped)
                        return noexcept(std::declval<Derived>().get().*std::declval<Signature Base::*>());
                    else
                        return noexcept((*std::declval<Derived>()).*std::declval<Signature Base::*>());
                }
            }
            static constexpr bool value = get_value();
        };

        /** @brief utility for extracting the base and signature types from a member class ptr */
        ///@{
        template <typename F> // base
        struct invoke_memptr_trais;

        template <typename Base, typename Signature>
        struct invoke_memptr_trais<Signature Base::*>
        {
            using base = Base;
            using signature = Signature;
        };
        ///@}

        // utility for checking if a member class func is nothrow
        template <typename F, typename ...Args>
        struct is_nothrow_memptr_invocable_helper {
            using traits = invoke_memptr_trais<F>;
            static constexpr bool value = 
                is_nothrow_memptr_invocable<
                    typename traits::base, 
                    typename traits::signature, 
                    Args...
                >::value;
        };



        template <typename Base, typename Signature, typename Derived, typename... Args>
        constexpr auto invoke_memptr(Signature Base::* member_func, Derived&& object, Args&&... args)
             noexcept(is_nothrow_memptr_invocable<Base, Signature, Derived, Args...>::value)
        {
            using object_t = std::decay_t<Derived>; // removes any qualifiers and references (only leaving the raw type or ptr)

            constexpr bool is_member_function = std::is_function_v<Signature>;
            constexpr bool is_wrapped = invoke_has_get_method<Derived>;
            constexpr bool is_derived_object = std::derived_from<object_t, Base>;
            
            if constexpr (is_member_function) 
                // its actually a member func
                if constexpr (is_derived_object) 
                    // regular object class or inherited from it, just the regular syntax
                    return (std::forward<Derived>(object).*member_func)(std::forward<Args>(args)...);
                else if constexpr (is_wrapped) 
                    // the object is inside a wrapper like std::reference_wrapper, call its get method
                    return (object.get().*member_func)(std::forward<Args>(args)...);
                else
                    // the object is surely a raw pointer or smart pointer or any class with the operator*() syntax
                    // could use the -> syntax like (*std::forward<Derived>(object)->*member_func) but idk
                    return ((*std::forward<Derived>(object)).*member_func)(std::forward<Args>(args)...);
            else
                // the object is actually a member variable
                if constexpr (is_derived_object)
                    return std::forward<Derived>(object).*member_func;
                else if constexpr (is_wrapped)
                    return object.get().*member_func;
                else
                    return (*std::forward<Derived>(object)).*member_func;
        }
    } // namespace detail

    /**
        @brief Same functionality as std::invoke
    */
    template <typename F, typename... Args>
    requires std::is_invocable_v<F, Args...>
    constexpr auto invoke(F&& f, Args&&... args) 
        noexcept(
            []() constexpr {
                if constexpr (std::is_member_pointer_v<std::decay_t<F>>)
                    return detail::is_nothrow_memptr_invocable_helper<F, Args...>::value;
                else
                    return std::is_nothrow_invocable_v<F, Args...>;
            }()
        )
        -> std::invoke_result_t<F, Args...>
    {
        if constexpr (std::is_member_pointer_v<std::decay_t<F>>)
            return detail::invoke_memptr(std::forward<F>(f), std::forward<Args>(args)...);
        else
            return std::forward<F>(f)(std::forward<Args>(args)...);
    }

    /**
        @brief Same functionality as std::invoke_r
    */
    template <typename Ret, typename F, typename... Args>
    requires std::is_invocable_r_v<Ret, F, Args...>
    constexpr Ret invoke_r(F&& f, Args&&... args) 
        noexcept(
            std::is_void_v<Ret> 
            ? noexcept(invoke(std::declval<F>(), std::declval<Args>()...))
            : noexcept(static_cast<Ret>(invoke(std::declval<F>(), std::declval<Args>()...)))
        )
    {
        if constexpr (std::is_void_v<Ret>)
            invoke(std::forward<F>(f), std::forward<Args>(args)...);
        else
            return static_cast<Ret>(invoke(std::forward<F>(f), std::forward<Args>(args)...));
    }
} // namespace jman