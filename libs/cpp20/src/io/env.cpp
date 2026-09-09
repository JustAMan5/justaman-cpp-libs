#include <justaman/io/env.hpp>
#include <justaman/memory/lifetime.hpp>
#include <justaman/errors/safe_exceptions.hpp>
#include <algorithm>
#include <filesystem>
#include <new>
#include <unordered_map>
#include <fstream>
namespace fs = std::filesystem;

// ################################################################################################
// ---------------------------------------- PRIVATE IMPL ----------------------------------------//
// ################################################################################################

struct jman::env_manager::__impl {
    std::unordered_map<std::string, std::string>    data {};
    fs::path                           m_loaded_path {};

    fs::path __find_env_upwards(const std::string& filename, int max_depth = 5) const;
};

fs::path jman::env_manager::__impl::__find_env_upwards(const std::string& filename, int max_depth) const {
    fs::path current = fs::current_path();

    for (int i = 0; i < max_depth; ++i) {
        fs::path candidate = current / filename;
        if  (
                fs::exists(candidate) 
                && fs::is_regular_file(candidate)
            )
            return candidate;

        if (!current.has_parent_path() || current == current.root_path())
             break;
        current = current.parent_path();
    }

    return "";
}

// ################################################################################################
// ---------------------------------------- CONSTRUCTORS ----------------------------------------//
// ################################################################################################


jman::env_manager::env_manager() 
{
    void* ptr = ::operator new(sizeof(__impl), std::align_val_t{alignof(__impl)});
    m_impl = jman::construct_at<__impl>(static_cast<__impl*>(ptr));
}
jman::env_manager::~env_manager() noexcept
{
    jman::destruct_at<__impl>(m_impl);
    ::operator delete(static_cast<void*>(m_impl), std::align_val_t{alignof(__impl)});
};

// ################################################################################################
// ----------------------------------------- LOADING ENV ----------------------------------------//
// ################################################################################################


bool jman::env_manager::load(std::string_view path)
{

}

bool jman::env_manager::load(const std::string& path)
{
    
}

// ################################################################################################
// -------------------------------------- CHECKING CACHE ----------------------------------------//
// ################################################################################################

bool jman::env_manager::has(std::string_view key) const
{
    return m_impl->data.find(key.data()) != m_impl->data.end();
}

bool jman::env_manager::has(const std::string& key) const
{
    return m_impl->data.find(key) != m_impl->data.end();
}




std::string_view jman::env_manager::get_value(std::string_view key, std::string_view fallback) const
{
    auto it = m_impl->data.find(key.data());
    if (it != m_impl->data.end())
        return it->second;

    return fallback;
}

std::string_view jman::env_manager::get_value(const std::string& key, std::string_view fallback) const
{
    auto it = m_impl->data.find(key);
    if (it != m_impl->data.end())
        return it->second;

    return fallback;
}




int jman::env_manager::get_int(std::string_view key, int fallback) const
{
    auto it = m_impl->data.find(key.data());
    if (it == m_impl->data.end()) 
        return fallback;

    JMAN_TRY { return std::stoi(it->second); } JMAN_CATCH (...) { return fallback; }
}

int jman::env_manager::get_int(const std::string& key, int fallback) const
{
    auto it = m_impl->data.find(key);
    if (it == m_impl->data.end()) 
        return fallback;
    
    JMAN_TRY { return std::stoi(it->second); } JMAN_CATCH (...) { return fallback; }
}




bool jman::env_manager::get_bool(std::string_view key, bool fallback) const
{
    auto it = m_impl->data.find(key.data());
    if (it == m_impl->data.end()) return fallback;
    
    std::string str = it->second;
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);

    if (str == "true" || str == "1" || str == "yes" || str == "on") return true;
    if (str == "false" || str == "0" || str == "no" || str == "off") return false;

    return fallback;
}

bool jman::env_manager::get_bool(const std::string& key, bool fallback) const
{
    auto it = m_impl->data.find(key);
    if (it == m_impl->data.end()) return fallback;
    
    std::string str = it->second;
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);

    if (str == "true" || str == "1" || str == "yes" || str == "on") return true;
    if (str == "false" || str == "0" || str == "no" || str == "off") return false;
    
    return fallback;
}