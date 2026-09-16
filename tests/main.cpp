#include "justaman/io/logger.hpp"
#include <justaman/containers/priority_queue.hpp>

int main() {
    jman::priority_queue<int> test {
    {1, 1},
    {2, 2},
    {3, 3},
    {5, 5},
    {4, 4},
    {7, 20},
    {8, 8},
    };

    test.push(50, -2);

    while (!test.empty()) {
        jman::println("popped {}", test.peek());
        test.pop();
    }
}