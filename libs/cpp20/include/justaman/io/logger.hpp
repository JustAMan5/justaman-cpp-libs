#pragma once

#include <concepts>
#include <justaman/utils/attributes.h>
#include <chrono>
#include <atomic>
#include <format>
#include <source_location>
#include <system_error>
#include <type_traits>
#include <utility>
#include <version> // forced to include it to avoid some weird problems when using std::println
#if defined (__cpp_lib_print) && __cpp_lib_print >= 202207L
#   define JMAN_USE_PRINT_FOR_LOGGER
#   include <print>
#else
#   include <iostream>
#endif

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

    struct log_location 
    {
        std::string_view file;
        std::uint_least32_t line;

        consteval log_location(std::source_location loc = std::source_location::current()) noexcept
            : file(extract_filename(loc.file_name())), line(loc.line()) {}

        private:
            static consteval std::string_view extract_filename(std::string_view path) noexcept {
                const std::size_t last_slash = path.find_last_of("/\\");
                return (last_slash == std::string_view::npos) ? path : path.substr(last_slash + 1);
            }
    };


    template <typename... Args>
    struct log_info
    {
        std::format_string<Args...> fmt;
        log_location loc;

        // default constructor captures the caller's location implicitly
        template <typename T> requires std::convertible_to<T, std::string_view>
        consteval log_info(const T& str, std::source_location current_loc = std::source_location::current()) noexcept
            : fmt(str), loc(current_loc) {}
    };

    // #############################################################################
    // --------------------------------- MAIN API --------------------------------//
    // #############################################################################

    template <typename... Args>
    JMAN_FORCE_INLINE void          print(std::format_string<Args...> str, Args&&... args);
    template <typename... Args>
    JMAN_FORCE_INLINE void          println(std::format_string<Args...> str, Args&&... args);

    // #############################################################################
    // --------------------------- EXCEPTIONS AND SUCH ---------------------------//
    // #############################################################################

    /** @brief prints information about the passed error code */
    inline void                     error(std::error_code ec, std::string_view msg = {}, log_location loc = log_location{});
    /** @brief prints information about the passed error code and calls std::abort() */
    inline void                     critical(std::error_code ec, std::string_view msg = {}, log_location loc = log_location{});

    /** @brief prints information about the passed exception */
    inline void                     error(const std::exception& ex, std::string_view msg = {}, log_location loc = log_location{});
    /** @brief prints information about the passed exception and calls std::terminate() */
    inline void                     critical(const std::exception& ex, std::string_view msg = {}, log_location loc = log_location{});

    // #############################################################################
    // ----------------------------- LOGGER CLASS API ----------------------------//
    // #############################################################################

    class logger 
    {
        public:
            JMAN_NO_DISCARD JMAN_FORCE_INLINE log_level     level() const noexcept  { return m_level_flag.load(std::memory_order_relaxed); }
            JMAN_FORCE_INLINE void                          set_level(log_level lvl) noexcept { m_level_flag.store(lvl, std::memory_order_relaxed); }


            template <typename... Args>
            JMAN_FORCE_INLINE void  println(std::type_identity_t<log_info<Args...>> info, Args&&... args)
                { m_log_impl(log_level::all, "INFO", std::move(info), std::forward<Args>(args)...); }
            template <typename... Args>
            JMAN_FORCE_INLINE void  warnln(std::type_identity_t<log_info<Args...>> info, Args&&... args)
                { m_log_impl(log_level::all, "WARN", std::move(info), std::forward<Args>(args)...); }
            template <typename... Args>
            JMAN_FORCE_INLINE void  debugln(std::type_identity_t<log_info<Args...>> info, Args&&... args)
                { m_log_impl(log_level::all, "DEBUG", std::move(info), std::forward<Args>(args)...); }

        private:
            std::atomic<log_level> m_level_flag {log_level::all};

            // T reffers to the type of the format (if its passed as a log_info or regular std::format_string)
            template <typename T, typename... Args>
            void m_log_impl(log_level lvl, std::string_view lvl_str, T&& first, Args&&... args);
    };
} // namespace jman

namespace jman 
{
// #############################################################################
// --------------------------------- MAIN API --------------------------------//
// #############################################################################

template <typename... Args>
JMAN_FORCE_INLINE void print(std::format_string<Args...> str, Args&&... args) 
{
#   if defined (JMAN_USE_PRINT_FOR_LOGGER)
        std::print(str, std::forward<Args>(args)...);
#   else
        std::cout << std::format(str, std::forward<Args>(args)...);
#   endif
}
template <typename... Args> 
JMAN_FORCE_INLINE void println(std::format_string<Args...> str, Args&&... args)
{        
#   if defined (JMAN_USE_PRINT_FOR_LOGGER)
        std::println(str, std::forward<Args>(args)...);
#   else
        std::cout << std::format(str, std::forward<Args>(args)...) << '\n';
#   endif
}

// #############################################################################
// --------------------------- EXCEPTIONS AND SUCH ---------------------------//
// #############################################################################

inline void error(std::error_code ec, std::string_view msg, log_location loc)
{
    auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    jman::println("[{:%T}] [ERROR] [{}:{}]\n   - Message: {}\n   - Error: {}", now, loc.file, loc.line, msg, ec.message());
}
inline void critical(std::error_code ec, std::string_view msg, log_location loc)
{
    jman::error(ec, msg, loc);
    std::abort();
}

inline void error(const std::exception& ex, std::string_view msg, log_location loc)
{
    const auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    jman::println("[{:%T}] [EXCEPTION] [{}:{}]\n   - Message: {}\n   - What: {}", now, loc.file, loc.line, msg, ex.what());
}
inline void critical(const std::exception& ex, std::string_view msg, log_location loc)
{
    jman::error(ex, msg, loc);
    std::terminate();
}

// #############################################################################
// ----------------------------- LOGGER CLASS API ----------------------------//
// #############################################################################

template <typename T, typename... Args>
void logger::m_log_impl(log_level lvl, std::string_view lvl_str, T&& first, Args&&... args)
{
    static_assert(std::same_as<T, log_info<Args...>>, "Message passed doesnt match for log_info<Args...>");
    if (level() > lvl) return;

    auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());

    jman::print("[{:%T}] [{}] [{}:{}] -> ", now, lvl_str, first.loc.file, first.loc.line);
    jman::println(first.fmt, std::forward<Args>(args)...);
}


}; // namespace jman


