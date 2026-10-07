#pragma once

#include <type_traits>

namespace jman::lockfree::detail 
{
    template <class T>
    class work_stealing_queue_stealer 
    {

    };
} // namespace jman::lockfree::detail


namespace jman::lockfree
{
    /**
        @brief 
            chase-lev work stealing queue
            lets workers push and pop from the top and other workers "steal" from the bottom
    */
    template <class T>
    class work_stealing_queue 
    {
        static_assert(!std::is_reference_v<T>, "Type T can't be a reference of any kind.");
        static_assert(!std::is_void_v<T>, "Type T can't be a of void type.");

        static_assert(std::is_nothrow_destructible_v<T>, "Type T must has it's destructor marked as noexcept for safety.");
        static_assert(std::is_copy_constructible_v<T>, "Type T must be copy constructible.");
        static_assert(std::is_copy_assignable_v<T>, "Type T must be copy assignable.");

        public:
            using value_type            = T;
            using size_type             = std::size_t;
            using difference_type       = std::ptrdiff_t;
            using reference             = T&;
            using const_reference       = const T&;
            using pointer               = T*;
            using const_pointer         = const T*;

            using stealer               = detail::work_stealing_queue_stealer<T>;

        work_stealing_queue();
        ~work_stealing_queue() noexcept;

        /** 
            any copy or move could cause data races, 
            im sure there r a lot of dirty little tricks 
            but i dont want to shoot myself on the foot just for copy and move operations 
            for a class that 99% of the time doesnt need these operations
        */
        ///@{
        work_stealing_queue(const work_stealing_queue&) = delete;
        work_stealing_queue& operator=(const work_stealing_queue&) = delete;

        work_stealing_queue(work_stealing_queue&&) noexcept = delete; 
        work_stealing_queue& operator=(work_stealing_queue&&) noexcept = delete; 
        ///@}

        void push();
        void pop();
        void steal();
        void empty();
        void size();
        void thief(); // grabs a ref from the stealer/thief obj in the class

    };
} // namespace jman::detail