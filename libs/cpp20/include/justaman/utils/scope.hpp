#pragma once

#include <type_traits>
#include <utility>
#include <justaman/meta/invoke.hpp>

namespace jman 
{
    /**
        @brief
            utility for exception handling that wraps around a cleanup function that runs during stack unwinding 
            on exceptions, when the scope_guard is no longer necessary cuz no exceptions appeard, the dismiss() func
            should be called to tell that the cleanup operations are no longer needed.
        @note
            A bit slower than regular try-catch block cuz of the object creation overhead but in some cases
            its better to use scope_guard for simplicity and avoid code duplication
    */
    template <typename F>
    requires std::is_invocable_v<F>
    class scope_guard 
    {
        F cleanup;
        mutable bool active = true;

        public:
            explicit scope_guard(F&& f) : cleanup(std::move(f)) {}
            ~scope_guard() noexcept { if (active) jman::invoke(cleanup); }
            
            scope_guard(const scope_guard&) = delete;
            scope_guard& operator=(const scope_guard&) = delete;
            scope_guard(scope_guard&&) = delete;
            scope_guard& operator=(scope_guard&&) = delete;

            void dismiss() noexcept { active = false; } 
    };  
}