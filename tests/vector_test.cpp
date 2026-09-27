#include <justaman/containers/vector.hpp>
#include <justaman/io/logger.hpp>

void test2()
{
    jman::vector<int> vec {1, 2, 3, 4, 5};
    for (int item: vec)
        jman::println("{}", item);
}