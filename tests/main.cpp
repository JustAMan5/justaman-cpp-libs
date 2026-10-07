#include <justaman/io/dotenv.hpp>
#include <justaman/io/logger.hpp>

int main() {
    if (auto env = jman::dotenv::parse(std::string(PROJECT_SRC_DIR) + "/.env"))
    {
        jman::println("PORT:                    {}",      env->get_or<int>("PORT", 6767));
        jman::println("DATABASE_URL:            {}",      env->get_view_or("DATABASE_URL", "sad fallback"));
        jman::println("JWT_SECRET:              {}",      env->get_view_or("JWT_SECRET", "sad fallback"));
        jman::println("DEBUG:                   {}",      env->get_or<bool>("DEBUG", false));
        jman::println("ENCRYPTION_KEY size_t:   {}",      env->get_or<std::size_t>("ENCRYPTION_KEY", 67));
        jman::println("ENCRYPTION_KEY int:      {}",      env->get_or<int>("ENCRYPTION_KEY", 67));
        jman::println("NO_QUOTED_TEXT:          {}",      env->get_or<std::string>("NO_QUOTED_TEXT", "not found bro"));
        
    }


    return 0;
}