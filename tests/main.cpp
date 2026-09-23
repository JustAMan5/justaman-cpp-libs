#include "justaman/containers/vector.hpp"
#include "justaman/io/logger.hpp"

// enum class dividing_error: unsigned char {
//     second_zero = 0,
// };

// jman::expected<float, dividing_error> divide(float a, float b)
// {
//     if (b == 0) return jman::unexpected{dividing_error::second_zero};
//     return a / b;
// }

int main() 
{
    jman::vector<float> list {1, 2, 3, 4, 5, 6, 7, 8};
    // list.reserve(50);
    // list.insert(2, 5, 1000);

    // for (auto val: list)
    //     jman::println("{}", val);

    // jman::println("----------------------------");

    list.erase(3, 6);
    jman::println("{}", list.size());

    for (auto val: list)
        jman::println("- {}", val);
}