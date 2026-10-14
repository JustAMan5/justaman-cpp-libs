#include <justaman/io/dotenv.hpp>
#include <fstream>
#include <charconv>

namespace jman 
{

// #######################################################################################
// ----------------------------------- PRIVATE METHODS ---------------------------------//
// #######################################################################################

inline std::string_view dotenv::m_trim(std::string_view str)
{
    auto start = str.find_first_not_of(" \t\n\r"); // get the index for the first real character that its not a space or tab (\t)
    if (start == std::string_view::npos) return "";
    auto end = str.find_last_not_of(" \t\n\r"); // get the index for the last real character that its not a space or tab (\t)
    return str.substr(start, end - start + 1);
}

template <typename T>
bool inline dotenv::m_try_parse_int(std::string_view str, T* value)
{
    if (str.empty()) return false;

    int base = 10;
    bool is_negative = false;

    if (str.front() == '-') {
        if constexpr (std::is_unsigned_v<T>) return false; // cannot be negative if its unsigned
        is_negative = true;
        str.remove_prefix(1);
    } else if (str.front() == '+')
        str.remove_prefix(1);

    // binary prefix
    if (str.size() > 2 && str[0] == '0' && (str[1] == 'b' || str[1] == 'B')) {
        base = 2;
        str.remove_prefix(2);
    }
    // hexadecimal prefix
    else if (str.size() > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        base = 16;
        str.remove_prefix(2);
    }

    auto end = str.end();
    auto [ptr, err] = std::from_chars(str.data(), end, *value, base);
    
    if (err == std::errc{} && ptr == end) {
        if (is_negative) *value = -(*value);
        return true;
    }
    return false;
}
bool inline dotenv::m_try_parse_double(std::string_view str, double* value)
{
    if (str.empty()) return false;

    auto end = str.end();
    auto [ptr, err] = std::from_chars(str.data(), end, *value);

    return err == std::errc{} && ptr == end;
}
inline dotenv::var_t dotenv::m_parse_string(std::string_view raw)
{
    // try std::size_t
    std::size_t sizet_val;
    if (m_try_parse_int<std::size_t>(raw, &sizet_val))
        return sizet_val;

    // try double
    double double_val;
    if (m_try_parse_double(raw, &double_val))
        return double_val;

    // fallback to just the string
    return std::string(raw);
}

// #######################################################################################
// ------------------------------------ PUBLIC METHODS ---------------------------------//
// #######################################################################################

bool dotenv::load(std::string_view filepath)
{
    unload();

    std::ifstream file(filepath.data());
    if (!file) return false;

    std::string current_line;
    while (std::getline(file, current_line))
    {
        current_line = m_trim(current_line);

        if (current_line.empty() || current_line[0] == '#') 
            continue; // skip if its a comment or empty line

        std::size_t separatorPos = current_line.find('=');
        if (separatorPos == std::string_view::npos) 
            continue; // skip if no '=' is found

        std::string_view key = m_trim(std::string_view(current_line).substr(0, separatorPos));
        std::string_view value = m_trim(std::string_view(current_line).substr(separatorPos + 1));

        std::size_t commentPos = value.find('#');
        if (commentPos != std::string_view::npos) {
            value = value.substr(0, commentPos);
            value = m_trim(value); // trim again for any remaining spaces or tabs
        }

        if (!value.empty() && (value[0] == '"' || value[0] == '\'')) 
        // value is a string with "" or '' characters found
        {
            value = value.substr(1, value.size() - 2);
            m_envVars[std::string(key)] = std::string(value);
        } else if (value == "on" || value == "high" || value == "true" ||
                   value == "ON" || value == "HIGH" || value == "TRUE") 
        // value is a boolean with true
        {
            m_envVars[std::string(key)] = true;
        } else if (value == "off" || value == "low" || value == "false" ||
            value == "OFF" || value == "LOW" || value == "FALSE") 
        // value is a boolean with false
        {
            m_envVars[std::string(key)] = false;
        } else 
        // value is of type string, int, float or size_t
        {
            m_envVars[std::string(key)] = m_parse_string(value);
        }
    }

    return true;
}

} // namespace jman