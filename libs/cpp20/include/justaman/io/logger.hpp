#pragma once

#include <justaman/utils/attributes.h>
#include <atomic>
#include <format>
#include <source_location>
#include <system_error>
#include <utility>
#include <version> // forced to include it to avoid some weird problems when using std::println
#if defined (__cpp_lib_print) && __cpp_lib_print >= 202207L
#   define JMAN_USE_PRINT_FOR_LOGGER
#   include <print>
#else
#   include <iostream>
#endif


namespace jman::detail
{
    struct logger_configs;
} // namespace jman::detail

namespace jman 
{
    enum class log_level: std::uint8_t 
    {
        all             = 0,
        debug           = 1,
        warn            = 2,
        exception       = 3,
        none            = 4,
    };

    inline constexpr log_level log_all =            log_level::all;
    inline constexpr log_level log_debug =          log_level::debug;
    inline constexpr log_level log_warn =           log_level::warn;
    inline constexpr log_level log_exception =      log_level::exception;
    inline constexpr log_level log_none =           log_level::none;

    template <typename... Args>
    struct log_info
    {
        std::format_string<Args...> fmt;
        std::source_location loc;

        // compile MUST construct the object at compile time
        template <typename T> requires std::convertible_to<T, std::string_view>
        consteval log_info(const T& str, std::source_location current_loc = std::source_location::current()) noexcept
            : fmt(str), loc(current_loc) {}
    };

    JMAN_NO_DISCARD JMAN_FORCE_INLINE detail::logger_configs& 
                                    configure_logs() noexcept;

    // #############################################################################
    // --------------------------------- MAIN API --------------------------------//
    // #############################################################################

    template <typename... Args>
    void                            println(log_info<Args...> info, Args&&... args);

    template <typename... Args>
    void                            debugln(log_info<Args...> info, Args&&... args);

    template <typename... Args>
    void                            warnln(log_info<Args...> info, Args&&... args);

    template <typename... Args>
    void                            errorln(log_info<Args...> info, Args&&... args);

    template <typename... Args>
    void                            criticalln(log_info<Args...> info, Args&&... args);

    // #############################################################################
    // ------------------------------ CONDITIONALS -------------------------------//
    // #############################################################################

    template <typename... Args>
    void                            print_if(bool condition, log_info<Args...> target, Args&&... args);

    template <typename... Args>
    void                            debug_if(bool condition, log_info<Args...> target, Args&&... args);

    template <typename... Args>
    void                            warn_if(bool condition, log_info<Args...> target, Args&&... args);

    template <typename... Args>
    void                            error_if(bool condition, log_info<Args...> target, Args&&... args);

    // #############################################################################
    // --------------------------- EXCEPTIONS AND SUCH ---------------------------//
    // #############################################################################

    inline void                     error(std::error_code ec, std::string_view msg = {}, std::source_location loc = std::source_location::current());
    inline void                     critical(std::error_code ec, std::string_view msg = {}, std::source_location loc = std::source_location::current());

    inline void                     error(const std::exception& ex, std::string_view msg = {}, std::source_location loc = std::source_location::current());
    inline void                     critical(const std::exception& ex, std::string_view msg = {}, std::source_location loc = std::source_location::current());
} // namespace jman


namespace jman::detail
{
    struct logger_configs 
    {
        public:
            static logger_configs& get() noexcept 
            {
                static logger_configs obj;
                return obj;
            }
        
            inline logger_configs& set_level(log_level level) noexcept;
            inline log_level get_level() const noexcept;

            // Configuración de formato (ej. true = "[14:32:10] [INFO] [main.cpp:42] -> mensaje")
            inline void enable_meta_info(bool enable) noexcept;
            inline bool meta_info_enabled() const noexcept;

            inline void enable_colors(bool enable) noexcept;
            inline void colors_enabled() const noexcept;

        private:
            std::atomic<log_level> m_level_flag {log_level::warn};
            
        private:
            logger_configs() = default;
    };
} // namespace jman::detail


// namespace jman 
// {

//     enum class log_level: std::uint8_t 
//     {
//         all             = 0,
//         debug           = 1,
//         warn            = 2,
//         exception       = 3,
//         none            = 4,
//     };

//     inline constexpr log_level log_all =            log_level::all;
//     inline constexpr log_level log_debug =          log_level::debug;
//     inline constexpr log_level log_warn =           log_level::warn;
//     inline constexpr log_level log_exception =      log_level::exception;
//     inline constexpr log_level log_none =           log_level::none;
// } // namespace jman



// namespace jman::detail
// {
//     struct logger_configs 
//     {
//         public:
//             static logger_configs& get() noexcept 
//             {
//                 static logger_configs obj;
//                 return obj;
//             }
        
//             logger_configs& set_level(log_level level) noexcept
//             {
//                 m_level_flag.store(level, std::memory_order_relaxed);
//                 return *this;
//             }

//             JMAN_FORCE_INLINE log_level get_level() const noexcept 
//                 { return m_level_flag.load(std::memory_order_relaxed); }

//         private:
//             std::atomic<log_level> m_level_flag {log_level::warn};
            
//         private:
//             logger_configs() = default;
//     };
// } // namespace jman::detail



// namespace jman 
// {


// JMAN_FORCE_INLINE detail::logger_configs& configure_logs() noexcept
// {
//     return detail::logger_configs::get();
// }

// // #########################################################################################
// // ------------------------------------- PRINT FUNCTIONS ----------------------------------//
// // #########################################################################################

// template <typename... Args>
// JMAN_FORCE_INLINE void print(std::format_string<Args...> str, Args&&... args) 
// {
//     if (log_level::none == configure_logs().get_level()) return;
// #   if defined (JMAN_USE_PRINT_FOR_LOGGER)
//         std::print(str, std::forward<Args>(args)...);
// #   else
//         std::cout << std::format(str, std::forward<Args>(args)...);
// #   endif
// }
// template <typename... Args> 
// JMAN_FORCE_INLINE void println(std::format_string<Args...> str, Args&&... args)
// {        
//     if (log_level::none == configure_logs().get_level()) return;
// #   if defined (JMAN_USE_PRINT_FOR_LOGGER)
//         std::println(str, std::forward<Args>(args)...);
// #   else
//         std::cout << std::format(str, std::forward<Args>(args)...) << '\n';
// #   endif
// }

// // #########################################################################################
// // ------------------------------------- WARN FUNCTIONS ----------------------------------//
// // #########################################################################################

// template <typename... Args> 
// JMAN_FORCE_INLINE void warn(std::format_string<Args...> str, Args&&... args)
// {
//     if (log_level::warn < configure_logs().get_level()) return;
// #   if defined (JMAN_USE_PRINT_FOR_LOGGER)
//         std::print("\e[0;33m[Warn] -> ");
//         std::print(str, std::forward<Args>(args)...);
//         std::print("\e[0m");
// #   else
//         std::cout << "\e[0;33m[Warn] -> " << std::format(str, std::forward<Args>(args)...) << "\e[0m";
// #   endif
// }   
// template <typename... Args>
// JMAN_FORCE_INLINE void warnln(std::format_string<Args...> str, Args&&... args)
// {
//     if (log_level::warn < configure_logs().get_level()) return;
// #   if defined (JMAN_USE_PRINT_FOR_LOGGER)
//         std::print("\e[0;33m[Warn] -> ");
//         std::print(str, std::forward<Args>(args)...);
//         std::println("\e[0m");
// #   else
//         std::cout << "\e[0;33m[Warn] -> " << std::format(str, std::forward<Args>(args)...) << "\e[0m\n";
// #   endif
// }   

// // ##########################################################################################
// // ------------------------------------- DEBUG FUNCTIONS ----------------------------------//
// // ##########################################################################################

// template <typename... Args>
// JMAN_FORCE_INLINE void debug(std::format_string<Args...> str, Args&&... args)
// {
//     if (log_level::debug < configure_logs().get_level()) return;
// #   if defined (JMAN_USE_PRINT_FOR_LOGGER)
//         std::print("\e[0;32m[Debug] -> ");
//         std::print(str, std::forward<Args>(args)...);
//         std::print("\e[0m");
// #   else
//         std::cout << "\e[0;32m[Debug] -> " << std::format(str, std::forward<Args>(args)...) << "\e[0m";
// #   endif
// }   
// template <typename... Args> 
// JMAN_FORCE_INLINE void debugln(std::format_string<Args...> str, Args&&... args)
// {
//     if (log_level::debug < configure_logs().get_level()) return;
// #   if defined (JMAN_USE_PRINT_FOR_LOGGER)
//         std::print("\e[0;32m[Debug] -> ");
//         std::print(str, std::forward<Args>(args)...);
//         std::println("\e[0m");
// #   else
//         std::cout << "\e[0;32m[Debug] -> " << std::format(str, std::forward<Args>(args)...) << "\e[0m\n";
// #   endif
// }  

// // #########################################################################################
// // ------------------------------------ ERROR FUNCTIONS ---------------------------------//
// // #########################################################################################

// template <typename... Args> 
// JMAN_FORCE_INLINE void error(std::format_string<Args...> str, Args&&... args) 
// {   
//     if (log_level::exception < configure_logs().get_level()) return;
// #   if defined (JMAN_USE_PRINT_FOR_LOGGER)
//         std::print("\e[0;31m[Error] -> ");
//         std::print(str, std::forward<Args>(args)...);
//         std::print("\e[0m");
// #   else
//         std::cout << "\e[0;31m[Error] -> " << std::format(str, std::forward<Args>(args)...) << "\e[0m";
// #   endif
// }
// template <typename... Args> 
// JMAN_FORCE_INLINE void errorln(std::format_string<Args...> str, Args&&... args) 
// {  
//     if (log_level::exception < configure_logs().get_level()) return;
// #   if defined (JMAN_USE_PRINT_FOR_LOGGER)
//         std::print("\e[0;31m[Error] -> ");
//         std::print(str, std::forward<Args>(args)...);
//         std::println("\e[0m");
// #   else
//         std::cout << "\e[0;31m[Error] -> " << std::format(str, std::forward<Args>(args)...) << "\e[0m\n";
// #   endif
// }

// JMAN_FORCE_INLINE void error(std::error_code code)
// {
//     if (log_level::exception < configure_logs().get_level()) return;
// #if defined (JMAN_USE_PRINT_FOR_LOGGER)
//         std::print(
//             "\e[0;31m[ Error ]\n"
//             "   Category : {}\n"
//             "   Value    : {}\n"
//             "   Message  : {}\e[0m\n",
//             code.category().name(),
//             code.value(),
//             code.message()
//         );
// #else
//         std::cout << std::format(
//             "\e[0;31m[ Error ]\n"
//             "   Category : {}\n"
//             "   Value    : {}\n"
//             "   Message  : {}\e[0m\n",
//             code.category().name(),
//             code.value(),
//             code.message()
//         );
// #endif
// }


// } // namespace jman