#pragma once

/**
    @TODO: MAKE PROPER IO (Input Output) LIBS WITH CUSTOM LIGHTWEIGHT AND FAST FORMAT LIKE emio or {fmt}
            but in a very far future
*/ 

#include <format>
#include <system_error>
#include "../utils/attributes.h"
#include <utility>
#include <version> // forced to include it to avoid some weird problems when using std::println
#if defined (__cpp_lib_print) && __cpp_lib_print >= 202207L
    #include <print>
#else
    #include <iostream>
#endif

// ############################################################################################
// ------------------------------------- LOGGER NAMESPACE ----------------------------------//
// ############################################################################################

namespace jman 
{
    enum class log_level: std::uint8_t 
    {
        debug,
        warn,
        exception,
        none,
    };

    inline constexpr log_level log_debug =          log_level::debug;
    inline constexpr log_level log_warn =           log_level::warn;
    inline constexpr log_level log_exception =      log_level::exception;
    inline constexpr log_level log_none =           log_level::none;

    namespace detail
    {
        struct logger_configs 
        {
            public:
            
                static logger_configs& get() noexcept 
                {
                    static logger_configs obj;
                    return obj;
                }
            
                logger_configs& set_level(log_level level) noexcept
                {
                    m_level_flag = level;
                    return *this;
                }

                JMAN_FORCE_INLINE log_level get_level() const noexcept 
                    { return m_level_flag; }

            private:
                log_level m_level_flag = log_level::warn;
                logger_configs() {}
        };
    }

    JMAN_FORCE_INLINE detail::logger_configs& configure_logs() noexcept
    {
        return detail::logger_configs::get();
    }

    // #########################################################################################
    // ------------------------------------- INFO FUNCTIONS ----------------------------------//
    // #########################################################################################

    template <typename... Args>
    JMAN_FORCE_INLINE void print(std::format_string<Args...> str, Args&&... args) 
    {
        if (log_level::none < configure_logs().get_level()) return;
        #if defined (__cpp_lib_print) && __cpp_lib_print >= 202207L
            std::print(std::move(str), std::forward<Args>(args)...);
        #else
            std::cout << std::format(std::move(str), std::forward<Args>(args)...);
        #endif
    }
    template <typename... Args> 
    JMAN_FORCE_INLINE void println(std::format_string<Args...> str, Args&&... args)
    {        
        if (log_level::none < configure_logs().get_level()) return;
        #if defined (__cpp_lib_print) && __cpp_lib_print >= 202207L
            std::println(std::move(str), std::forward<Args>(args)...);
        #else
            std::cout << std::format(std::move(str), std::forward<Args>(args)...) << '\n';
        #endif
    }

    // #########################################################################################
    // ------------------------------------- WARN FUNCTIONS ----------------------------------//
    // #########################################################################################

    template <typename... Args> 
    JMAN_FORCE_INLINE void warn(std::format_string<Args...> str, Args&&... args)
    {
        if (log_level::warn < configure_logs().get_level()) return;
        #if defined (__cpp_lib_print) && __cpp_lib_print >= 202207L
            std::print("\e[0;33m[Warn] -> ");
            std::print(std::move(str), std::forward<Args>(args)...);
            std::print("\e[0m");
        #else
            std::cout << "\e[0;33m[Warn] -> " << std::format(std::move(str), std::forward<Args>(args)...) << "\e[0m";
        #endif
    }   
    template <typename... Args>
    JMAN_FORCE_INLINE void warnln(std::format_string<Args...> str, Args&&... args)
    {
        if (log_level::warn < configure_logs().get_level()) return;
        #if defined (__cpp_lib_print) && __cpp_lib_print >= 202207L
            std::print("\e[0;33m[Warn] -> ");
            std::print(std::move(str), std::forward<Args>(args)...);
            std::println("\e[0m");
        #else
            std::cout << "\e[0;33m[Warn] -> " << std::format(std::move(str), std::forward<Args>(args)...) << "\e[0m\n";
        #endif
    }   

    // ##########################################################################################
    // ------------------------------------- DEBUG FUNCTIONS ----------------------------------//
    // ##########################################################################################

    template <typename... Args>
    JMAN_FORCE_INLINE void debug(std::format_string<Args...> str, Args&&... args)
    {
        if (log_level::debug < configure_logs().get_level()) return;
        #if defined (__cpp_lib_print) && __cpp_lib_print >= 202207L
            std::print("\e[0;32m[Debug] -> ");
            std::print(std::move(str), std::forward<Args>(args)...);
            std::print("\e[0m");
        #else
            std::cout << "\e[0;32m[Debug] -> " << std::format(std::move(str), std::forward<Args>(args)...) << "\e[0m";
        #endif
    }   
    template <typename... Args> 
    JMAN_FORCE_INLINE void debugln(std::format_string<Args...> str, Args&&... args)
    {
        if (log_level::debug < configure_logs().get_level()) return;
        #if defined (__cpp_lib_print) && __cpp_lib_print >= 202207L
            std::print("\e[0;32m[Debug] -> ");
            std::print(std::move(str), std::forward<Args>(args)...);
            std::println("\e[0m");
        #else
            std::cout << "\e[0;32m[Debug] -> " << std::format(std::move(str), std::forward<Args>(args)...) << "\e[0m\n";
        #endif
    }  

    // #########################################################################################
    // ------------------------------------ ERROR FUNCTIONS ---------------------------------//
    // #########################################################################################

    template <typename... Args> 
    JMAN_FORCE_INLINE void error(std::format_string<Args...> str, Args&&... args) 
    {   
        if (log_level::exception < configure_logs().get_level()) return;
        #if defined (__cpp_lib_print) && __cpp_lib_print >= 202207L
            std::print("\e[0;31m[Error] -> ");
            std::print(std::move(str), std::forward<Args>(args)...);
            std::print("\e[0m");
        #else
            std::cout << "\e[0;31m[Error] -> " << std::format(std::move(str), std::forward<Args>(args)...) << "\e[0m";
        #endif
    }
    template <typename... Args> 
    JMAN_FORCE_INLINE void errorln(std::format_string<Args...> str, Args&&... args) 
    {  
        if (log_level::exception < configure_logs().get_level()) return;
        #if defined (__cpp_lib_print) && __cpp_lib_print >= 202207L
            std::print("\e[0;31m[Error] -> ");
            std::print(std::move(str), std::forward<Args>(args)...);
            std::println("\e[0m");
        #else
            std::cout << "\e[0;31m[Error] -> " << std::format(std::move(str), std::forward<Args>(args)...) << "\e[0m\n";
        #endif
    }
    
    JMAN_FORCE_INLINE void error(std::error_code code)
    {
        if (log_level::exception < configure_logs().get_level()) return;
        #if defined (__cpp_lib_print) && __cpp_lib_print >= 202207L
            std::print(
                "\e[0;31m[ Error ]\n"
                "   Category : {}\n"
                "   Value    : {}\n"
                "   Message  : {}\e[0m\n",
                code.category().name(),
                code.value(),
                code.message()
            );
        #else
            std::cout << std::format(
                "\e[0;31m[ Error ]\n"
                "   Category : {}\n"
                "   Value    : {}\n"
                "   Message  : {}\e[0m\n",
                code.category().name(),
                code.value(),
                code.message()
            );
        #endif
    }
} // namespace jman