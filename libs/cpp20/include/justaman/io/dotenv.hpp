#pragma once

#include <justaman/utils/attributes.h>
#include <justaman/meta/type_list.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>

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
    class dotenv 
    {
        private:
            template <typename T>
            using return_type_t = std::conditional_t<std::is_trivially_copyable_v<T>, T, const T&>;

        public:
            // all supported types
            using supported_types_t = type_list<
                std::string, 
                std::size_t,
                bool, 
                double
            >;

            using var_t = jman::unpack_t<supported_types_t, std::variant>;
    
            dotenv() = default; // not loaded
            JMAN_NO_DISCARD static inline std::optional<dotenv>         parse(std::string_view filepath) noexcept;

            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool                      empty() const noexcept { return m_envVars.empty(); }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE std::size_t               size() const noexcept { return m_envVars.size(); }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE bool                      exists(std::string_view key) const noexcept;

            template <typename T>
            requires jman::contains_v<T, supported_types_t>
            JMAN_NO_DISCARD return_type_t<T>                            get_or(std::string_view key, const T& fallback) const;
            JMAN_NO_DISCARD inline std::string_view                     get_view_or(std::string_view key, std::string_view fallback) const;

            template <typename T>
            requires jman::contains_v<T, supported_types_t>
            JMAN_NO_DISCARD std::optional<T>                            get(std::string_view key) const;
            JMAN_NO_DISCARD inline std::optional<std::string_view>      get_view(std::string_view key) const;

            JMAN_FORCE_INLINE void                                      unload() noexcept { m_envVars.clear(); }
            bool                                                        load(std::string_view filepath);

        private:
            std::unordered_map<
                std::string, 
                var_t, 
                detail::transparent_string_hash, 
                std::equal_to<void>
            >  m_envVars; // with this custom hash and equal_to<void> we tell the compiler that for searching dont create a new std::string and 
                          // instead compute the hash directly from a std::string_view and do everything else 
        
        private:
            // removes spaces and tabs from the sides (not in between of the =)
            static std::string_view      m_trim(std::string_view str);

            template <typename T>
            static bool                  m_try_parse_int(std::string_view str, T* value);
            static bool                  m_try_parse_double(std::string_view str, double* value);
            static var_t                 m_parse_string(std::string_view raw);
    };     
    
    template <class Config>
    class dotenv_config
    {
        static_assert(std::is_nothrow_destructible_v<Config>, "Config type must be nothrow destructible for safety.");
        static_assert(std::is_constructible_v<Config, const dotenv&>, "Config must be constructible from 'const dotenv&'.");

        public:
            dotenv_config(std::string_view filepath)
                : m_manager(dotenv::parse(filepath).value_or(dotenv{})), m_holder(m_manager) {}
            
            // prevebt ussyes if Config holds a ref to the manager internally
            dotenv_config(const dotenv_config&) = delete;
            dotenv_config& operator=(const dotenv_config&) = delete;

            dotenv_config(dotenv_config&&) noexcept = default;
            dotenv_config& operator=(dotenv_config&&) noexcept = default;


            JMAN_NO_DISCARD JMAN_FORCE_INLINE const Config& operator*() const noexcept  { return m_holder; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE const Config* operator->() const noexcept { return &m_holder; }
            
            JMAN_NO_DISCARD JMAN_FORCE_INLINE Config& operator*() noexcept  { return m_holder; }
            JMAN_NO_DISCARD JMAN_FORCE_INLINE Config* operator->() noexcept { return &m_holder; }

            JMAN_NO_DISCARD JMAN_FORCE_INLINE const dotenv& raw_env() const noexcept { return m_manager; }

        private:
            // must be on this order so m_manager initializes first and we avoid the compiler from fucking the code
            dotenv m_manager {}; // env variables manager
            Config m_holder {}; // config wrapper
    };
} // namespace jman

namespace jman 
{

JMAN_NO_DISCARD inline std::optional<dotenv> dotenv::parse(std::string_view filepath) noexcept
{
    dotenv instance;
    if (instance.load(filepath)) return instance;
    return std::nullopt;
}

// #######################################################################################
// -------------------------------------- GETTERS --------------------------------------//
// #######################################################################################

JMAN_NO_DISCARD JMAN_FORCE_INLINE bool dotenv::exists(std::string_view key) const noexcept
{
    return m_envVars.find(key) != m_envVars.end();
}

template <typename T>
requires jman::contains_v<T, dotenv::supported_types_t>
dotenv::return_type_t<T> dotenv::get_or(std::string_view key, const T& fallback) const
{
    if (key.empty()) return fallback;

    auto it = m_envVars.find(key);
    if (it != m_envVars.end() && std::holds_alternative<T>(it->second))
        return std::get<T>(it->second);
    
    return fallback;
}
inline std::string_view dotenv::get_view_or(std::string_view key, std::string_view fallback) const
{
    if (key.empty()) return fallback;

    auto it = m_envVars.find(key);
    if (it == m_envVars.end()) 
        return fallback;
    
    if (std::holds_alternative<std::string>(it->second))
        return std::string_view{std::get<std::string>(it->second)};
    
    return fallback;
}

template <typename T>
requires jman::contains_v<T, dotenv::supported_types_t>
std::optional<T> dotenv::get(std::string_view key) const
{
    if (key.empty()) return std::nullopt;

    auto it = m_envVars.find(key);
    if (it == m_envVars.end()) 
        return std::nullopt;

    
    if (std::holds_alternative<T>(it->second))
        return std::get<T>(it->second);
    
    return std::nullopt;
}
inline std::optional<std::string_view> dotenv::get_view(std::string_view key) const
{
    if (key.empty()) return std::nullopt;

    auto it = m_envVars.find(key);
    if (it == m_envVars.end()) 
        return std::nullopt;

    if (std::holds_alternative<std::string>(it->second))
        return std::string_view{std::get<std::string>(it->second)};
        
    return std::nullopt;
}

} // namespace jman