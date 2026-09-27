#pragma once

#include <cstdint>
namespace jman 
{

    class jobs_manager;
    class job_context;
    struct job_handle;

    using job_group = job_handle;
    using job_function = void(*)(job_context&, void*);

    /**
        @brief
            helpful to understand at what state is the given job
    */
    enum class job_state: std::uint8_t
    {
        invalid,
        waiting,
        ready,
        running,
        waiting_for_children,
        completed,
        cancelled
    };

} // namespace jman