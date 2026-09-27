#include <justaman/io/dotenv.hpp>
#include <fstream>

namespace jman 
{


bool dotenv::load(std::string_view  filepath)
{
    unload();

    std::ifstream file(filepath.data());
    if (!file) return false;

    std::string current_line;
    while (std::getline(file, current_line))
    {
        current_line = m_trim(current_line);
        if (current_line.empty() || current_line[0] == '#') continue; // skip if its a comment or empty line

        std::size_t separatorPos = current_line.find('=');
        if (separatorPos == std::string_view::npos) continue; // skip if no '=' is found

        std::string_view key = m_trim(current_line.substr(0, separatorPos));
        std::string_view value = m_trim(current_line.substr(separatorPos + 1));

        if (!value.empty() && (value[0] == '"' || value[0] == '\'')) 
        // value is a string "" or ' characters found
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