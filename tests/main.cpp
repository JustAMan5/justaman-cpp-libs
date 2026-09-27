#include <justaman/io/dotenv.hpp>
#include <justaman/io/logger.hpp>


void test() {
    jman::dotenv env{};
    jman::println("loaded: {}",         env.load(std::string(PROJECT_SRC_DIR) + "/.env"));
    jman::println("PORT: {}",           env.get<int>("PORT", 6767));
    jman::println("DATABASE_URL: {}",   env.get_view("DATABASE_URL", "sad fallback"));
    jman::println("JWT_SECRET: {}",     env.get_view("JWT_SECRET", "sad fallback"));
    jman::println("DEBUG: {}",          env.get<bool>("DEBUG", false));
    jman::println("ENCRYPTION_KEY: {}", env.get<std::size_t>("ENCRYPTION_KEY", 67));
    jman::println("NO_QUOTED_TEXT: {}", env.get<std::string>("NO_QUOTED_TEXT", "not found bro"));
}