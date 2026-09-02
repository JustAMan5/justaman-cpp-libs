#include <justaman/algorithms/searching.hpp>
#include <justaman/io/logger.hpp>
#include <vector>

int main()
{
    std::vector<int> test {1, 2, 3, 1, 5, 7, 2};

    jman::count(test.begin(), test.end(), [](const int& va){
        return va == 1;
    });
    
    return 0;
}