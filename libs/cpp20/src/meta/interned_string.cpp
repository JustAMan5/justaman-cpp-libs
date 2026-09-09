#include <justaman/meta/interned_string.hpp>

std::string_view jman::string_pool::intern(const std::string& str)
{
    auto [it, insreted] = m_set.insert(str);
    return *it;
}

std::string_view jman::string_pool::intern(std::string&& str)
{
    auto [it, insreted] = m_set.insert(str);
    return *it;
}

std::string_view jman::string_pool::intern(std::string_view str)
{
    auto [it, insreted] = m_set.insert(std::string(str));
    return *it;
}

void jman::string_pool::remove(const std::string& str)
{
    m_set.erase(str);
}

void jman::string_pool::remove(std::string_view str)
{
    m_set.erase(std::string(str));
}

void jman::string_pool::clear() noexcept
{
    m_set.clear();
}