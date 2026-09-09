#pragma once

#include <string_view>
#include <string>

namespace jman 
{
    class env_manager 
    {
        struct __impl;
        __impl* m_impl;

        private:
            env_manager();

        public:
            ~env_manager() noexcept;

            env_manager(const env_manager&) = delete;
            env_manager& operator=(const env_manager&) = delete;

            env_manager(env_manager&&) noexcept = delete;
            env_manager& operator=(env_manager&&) noexcept = delete;

            static env_manager& get() 
            {
                static env_manager manager {};
                return manager;
            }

            bool                    load(std::string_view path = "");
            bool                    load(const std::string& path = "");
            bool                    has(std::string_view path = "") const;
            bool                    has(const std::string& path = "") const;

            std::string_view        get_value(std::string_view key, std::string_view fallback = "") const;
            std::string_view        get_value(const std::string& key, std::string_view fallback = "") const;
            int                     get_int(std::string_view, int fallback = 0) const;
            int                     get_int(const std::string&, int fallback = 0) const;
            bool                    get_bool(std::string_view, bool fallback = false) const;
            bool                    get_bool(const std::string&, bool fallback = false) const;
    };
}