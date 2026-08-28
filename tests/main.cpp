#include <justaman/io/logger.hpp>
#include <justaman/utils/random.hpp>
#include <justaman/containers/flat_umap.hpp>
#include <justaman/meta/type_traits.hpp>

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
    bool test2 = jman::is_specialization_of_v<std::vector, jman::vector<int>>;
    jman::println("{}", test2);
    jman::println("{}", jman::is_specialization_of_v<jman::vector, jman::vector<int>>);
    
    
    // jman::random32 rng {};
    // jman::flat_umap<int, TestObject> test {};

    // test.emplace(1);
    // jman::println("1");
    // test.emplace(2);
    // jman::println("2");
    // test.emplace(3);
    // jman::println("3");
    // test.emplace(4);
    // jman::println("4");
    // test.emplace(5);
    // jman::println("5");
}