#include <justaman/io/logger.hpp>
#include <justaman/containers/vector.hpp>

int main()
{
    // std::vector<float> test {1, 2, 3, 1, 5, 7, 2};

    // jman::count(test.begin(), test.end(), [](const int& va){
    //     return va == 1;
    // });
    jman::vector<float> list = {1, 8, 5, 2, 8, 1, 5, 2, 8, 52, 2, 6, 1000};
    list.erase(2);
    
    for (auto num: list)
    {
        jman::println("list {}", num);
    };


    return 0;
}