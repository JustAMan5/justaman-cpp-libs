#include <justaman/io/dotenv.hpp>
#include <fstream>

namespace jman 
{


bool dotenv::load(std::string_view  filepath)
{
    std::ifstream file(filepath.data());
    if (!file) return false;

    std::string current_line;
    while (std::getline(file, current_line))
    {
        current_line = m_trim(current_line);
        if (current_line.empty() || current_line[0] == '#') continue; // skip if its a comment or empty line

        std::size_t separatorPos = current_line.find('=');
        if (separatorPos == std::string::npos) continue; // skip if no '=' is found

        std::string key = m_trim(current_line.substr(0, separatorPos));
        std::string value = m_trim(current_line.substr(separatorPos + 1));

        if (!value.empty() && (value[0] == '"' || value[0] == '\''))
            value = value.substr(1, value.size() - 2);
    }

    return true;
}

} // namespace jman