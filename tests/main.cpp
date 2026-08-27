#include "justaman/io/logger.hpp"
#include <justaman/utils/random.hpp>
#include <justaman/containers/flat_umap.hpp>

struct TestObject 
{
    int t = 0;
    TestObject() 
    {
        jman::println("construct");
    }
    ~TestObject() 
    {
        jman::println("destruct");
    }

    TestObject(const TestObject&) {
        jman::println("copy");
    }

    TestObject& operator=(const TestObject& other) {
        if (this != &other)
        {
            jman::println("copy2");
        }

        return *this;
    }

    TestObject(TestObject&&) noexcept {
        jman::println("move");
    }

    TestObject& operator=(TestObject&& other) noexcept {
        if (this != &other)
        {
            jman::println("move2");
        }
        return *this;
    }
};

int main()
{
    jman::random32 rng {};
    jman::flat_umap<int, TestObject> test {};

    test.emplace(1);
    jman::println("1");
    test.emplace(2);
    jman::println("2");
    test.emplace(3);
    jman::println("3");
    test.emplace(4);
    jman::println("4");
    test.emplace(5);
    jman::println("5");
}