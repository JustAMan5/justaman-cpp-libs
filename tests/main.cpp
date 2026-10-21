#include <cstdint>
#include <justaman/meta/type_list.hpp>
#include <justaman/errors/errors.hpp>

enum class ErrorTypes: std::uint8_t
{
    ConnectionFailed, 
    TimeoutReached,
};

jman::result<std::uint16_t> get_sock_id(bool will_fail)
{
    if (will_fail)
        return jman::new_error(ErrorTypes::TimeoutReached);

    return 5;
}

int main()
{
    auto result = get_sock_id(true);

}