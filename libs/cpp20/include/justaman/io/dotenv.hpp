#pragma once

#include <cstddef>
#include <justaman/utils/attributes.h>
#include <justaman/errors/excepted.hpp>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <cstdint>

namespace jman::detail 
{
    // for heterogeneous lookups
    struct transparent_string_hash {
        using is_transparent = void;

        std::size_t operator()(std::string_view str) const noexcept {
            return std::hash<std::string_view>{}(str);
        }
        
        std::size_t operator()(const std::string& str) const noexcept {
            return std::hash<std::string>{}(str);
        }
    };
}

namespace jman 
{
    enum class dotenv_error: std::uint8_t
    {
        invalid_key,
        invalid_get_type,
    };

    class dotenv 
    {
        public:
            dotenv() = default; // not loaded
            explicit dotenv(std::string_view filepath) { load(filepath); }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool              empty() const noexcept { return m_envVars.empty(); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t       size() const noexcept { return m_envVars.size(); }

            JMAN_NO_DISCARD inline bool                         exists(std::string_view key) const noexcept;

            template <typename T>
            const T&                                            get(std::string_view key, const T& fallback) const;
            inline std::string_view                             get_view(std::string_view key, std::string_view fallback) const;

            template <typename T>
            expected<T, dotenv_error>                           get(std::string_view key) const;
            inline expected<std::string_view, dotenv_error>     get_view(std::string_view key) const;

            JMAN_FORCE_INLINE void                              unload() noexcept { m_envVars.clear(); }
            bool                                                load(std::string_view filepath);

        private:
            using env_var_t = std::variant< // all supported types
                std::string, 
                std::size_t, // for more complex numbs like encryption keys and that kind of stuff
                int, // for stuff like ports ids
                bool, 
                double
            >;
            std::unordered_map<
                std::string, 
                env_var_t, 
                detail::transparent_string_hash, 
                std::equal_to<void>
            >  m_envVars; // with this custom hash and equal_to<void> we tell the compiler that for searching dont create a new std::string and 
                          // instead compute the hash directly from a std::string_view and do everything else 
        
        private:
            // removes spaces and tabs from the sides (not in between of the =)
            static inline std::string_view      m_trim(std::string_view str);

            template <typename T>
            static inline bool                  m_try_parse_int(std::string_view str, T* value);
            static inline bool                  m_try_parse_double(std::string_view str, double* value);
            static inline env_var_t             m_parse_string(std::string_view raw);
    };     
    
    template <class Config>
    class dotenv_config
    {
        static_assert(std::is_nothrow_destructible_v<Config>, "Config class type must be nothrow destructible for safety.");

        public:
            dotenv_config();

            Config& operator*() noexcept;
            Config* operator->() noexcept;
        

        private:
            dotenv m_manager {}; // env variables manager
            Config m_holder {}; // config wrapper
    };
} // namespace jman

namespace jman 
{

// #######################################################################################
// -------------------------------------- GETTERS --------------------------------------//
// #######################################################################################

JMAN_NO_DISCARD inline bool dotenv::exists(std::string_view key) const noexcept
{
    auto it = m_envVars.find(key);
    return it != m_envVars.end();
}

template <typename T>
const T& dotenv::get(std::string_view key, const T& fallback) const
{
    static_assert(
        std::same_as<T, std::string> || 
        std::same_as<T, int> ||
        std::same_as<T, double> ||
        std::same_as<T, bool> ||
        std::same_as<T, std::size_t>
    );

    auto it = m_envVars.find(key);
    if (it == m_envVars.end()) 
        return fallback;

    
    
    if (std::holds_alternative<T>(it->second))
        return std::get<T>(it->second);
    
    return fallback;
}
inline std::string_view dotenv::get_view(std::string_view key, std::string_view fallback) const
{
    auto it = m_envVars.find(key);
    if (it == m_envVars.end()) 
        return fallback;
    
    if (std::holds_alternative<std::string>(it->second))
        return std::get<std::string>(it->second);

    if (const char* env_fallback = std::getenv(key.data()))
        return std::string_view(env_fallback);
    
    return fallback;
}

template <typename T>
expected<T, dotenv_error> dotenv::get(std::string_view key) const
{
    static_assert(
        std::same_as<T, std::string> || 
        std::same_as<T, int> ||
        std::same_as<T, double> ||
        std::same_as<T, bool> ||
        std::same_as<T, std::size_t>
    );

    auto it = m_envVars.find(key);
    if (it == m_envVars.end()) 
        return jman::unexpected(dotenv_error::invalid_key);
    
    if (std::holds_alternative<T>(it->second))
        return std::get<T>(it->second);
    
    return jman::unexpected(dotenv_error::invalid_get_type);
}
inline expected<std::string_view, dotenv_error> dotenv::get_view(std::string_view key) const
{
    auto it = m_envVars.find(key);
    if (it == m_envVars.end()) 
        return jman::unexpected(dotenv_error::invalid_key);
    
    if (std::holds_alternative<std::string>(it->second))
        return std::get<std::string>(it->second);

    if (const char* env_fallback = std::getenv(key.data()))
        return std::string_view(env_fallback);
    
    return jman::unexpected(dotenv_error::invalid_get_type);
}

} // namespace jman