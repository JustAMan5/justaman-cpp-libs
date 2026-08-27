#pragma once

#include <cstddef>
#include <cstdint>
#include <justaman/utils/attributes.h>
#include <span>
#include <stop_token>

namespace jman 
{
    namespace __detail::jobs // detail impl (only defined in the cpp file)
    {
        class worker;
        struct job;
    }
    
    class jobs_manager;
    class job_context;
    struct job_handle;

    using job_group = job_handle; // will use same trick of jarchs to differentiate between a regular handle and a group
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

    inline constexpr job_state job_invalid =                job_state::invalid;
    inline constexpr job_state job_waiting =                job_state::waiting;
    inline constexpr job_state job_ready =                  job_state::ready;
    inline constexpr job_state job_running =                job_state::running;
    inline constexpr job_state job_waiting_for_children =   job_state::waiting_for_children;
    inline constexpr job_state job_completed =              job_state::completed;
    inline constexpr job_state job_cancelled =              job_state::cancelled;

    /**
        @brief
            interface that wraps an id, gen and a is_grup flag in a single 64 bits int

        @note   
            group flag takes the last 64th byte, so the remaining 31 on the right are for the gen, the rest for the id
    */
    struct job_handle 
    {
        public:
            using job_id_t = std::uint32_t;
            using job_gen_t = std::uint32_t;

            static constexpr job_id_t INVALID_ID = 0; 
            static constexpr std::uint64_t GROUP_FLAG = 1ULL << 63; // reserve last bit to differentiate between a regular handle and a group

            JMAN_NO_DISCARD constexpr job_id_t JMAN_FORCE_INLINE id() const noexcept { return static_cast<job_id_t>(m_value & 0xFFFFFFFF);}
            JMAN_NO_DISCARD constexpr job_gen_t JMAN_FORCE_INLINE gen() const noexcept { return static_cast<job_gen_t>((m_value & ~GROUP_FLAG) >> 32); }
            JMAN_NO_DISCARD constexpr std::uint64_t JMAN_FORCE_INLINE raw() const noexcept { return m_value; }

            JMAN_NO_DISCARD constexpr JMAN_FORCE_INLINE bool operator==(const job_handle& other) const noexcept { return id() == other.id() && gen() == other.gen(); }
            JMAN_NO_DISCARD constexpr JMAN_FORCE_INLINE bool operator!=(const job_handle& other) const noexcept { return id() != other.id() || gen() != other.gen(); }
            JMAN_NO_DISCARD constexpr JMAN_FORCE_INLINE bool valid() { return id() != INVALID_ID; }
            JMAN_NO_DISCARD constexpr JMAN_FORCE_INLINE bool is_group() { return m_value & GROUP_FLAG; }

            constexpr job_handle() = default;
            constexpr job_handle(job_id_t id, job_gen_t gen, bool is_group)
                : m_value((is_group ? GROUP_FLAG : 0) | (static_cast<std::uint64_t>(gen) << 32) | id) {};
            constexpr job_handle(std::uint64_t raw): m_value(raw) {};

        private:
            std::uint64_t m_value = 0;
    };

    class job_context 
    {

        public:
            JMAN_NO_DISCARD jobs_manager& manager() noexcept;
            JMAN_NO_DISCARD job_handle self() const noexcept;
            JMAN_NO_DISCARD std::uint32_t worker_index() const noexcept;
            JMAN_NO_DISCARD std::stop_token stop_token() const noexcept;
            JMAN_NO_DISCARD bool cancelled() const noexcept;

            template<typename T, typename... Args>
            JMAN_NO_DISCARD job_handle spawn_child(Args&&... args);
            bool add_dependency(job_handle dependency);
            void add_to_group(job_group group, job_handle job);
    };
    

    class jobs_manager 
    {
        friend class __detail::jobs::worker;
        friend class job_context;

        struct impl;
        std::unique_ptr<impl> m_impl;

        private:
            struct task_metadata {
                void* storage;

                void(*execute)(job_context&, void*);
                void(*destroy)(job_context&, void*);
            };
            
            template<typename F, typename... Args>
            void make_callable(F&& func, Args&&... args);
            job_handle submit_internal(task_metadata&& data);


        public:
            /**
                @param workerCount: default 0, std::thread::hardware_concurrency 
            */
            explicit jobs_manager(std::size_t workerCount = 0);
            ~jobs_manager() noexcept;

            jobs_manager(const jobs_manager&) = delete;
            jobs_manager& operator=(const jobs_manager&) = delete;

            jobs_manager(jobs_manager&&) = delete;
            jobs_manager& operator=(jobs_manager&&) = delete;

            // ------ modifiers
            void shutdown();

            // ------ specific struct callables
            template<typename T, typename... Args>
            job_handle submit(Args&&... args);
            template<typename T, typename... Args>
            job_handle submit_after(job_handle dependency, Args&&... args);
            template<typename T, typename... Args>
            job_handle submit_after(std::span<const job_handle> dependencies, Args&&... args);
            
            // ------ lambdas and other type of callables
            template<typename F, typename... Args>
            job_handle submit(F&& func, Args&&... args);
            template<typename F, typename... Args>
            job_handle submit_after(F&& func, job_handle dependency, Args&&... args);
            template<typename F, typename... Args>
            job_handle submit_after(F&& func, std::span<const job_handle> dependencies, Args&&... args);

            // ----- manager attributes
            JMAN_NO_DISCARD std::uint32_t workers() const noexcept;
            JMAN_NO_DISCARD bool is_worker_thread() const noexcept;
            JMAN_NO_DISCARD std::uint32_t current_worker_index() const noexcept;
            
            //-------- job handles attributes
            JMAN_NO_DISCARD bool exists(job_handle handle) const noexcept;
            JMAN_NO_DISCARD bool cancel(job_handle job);
            JMAN_NO_DISCARD job_state state(job_handle job) const noexcept;
            JMAN_NO_DISCARD bool completed(job_handle job) const noexcept;

            // ------- handle utils
            bool add_dependency(job_handle job, job_handle dependency);
            bool add_dependencies(job_handle job, std::span<const job_handle> dependencies);
            void wait(job_handle job);
            void wait(std::span<const job_handle> jobs);

            // ------ groups
            job_group create_group();
            void destroy_group(job_group group);

            // ------ utils
            void add_to_group(job_group group, job_handle job);
            JMAN_NO_DISCARD bool group_completed(job_group group) const noexcept;
            void wait_group(job_group group);

            // ------ specific struct callables for groups
            template<typename T, typename... Args>
            job_handle submit_to_group(job_group group, Args&&... args);
            template<typename T, typename... Args>
            job_handle submit_to_group_with_dependency(job_group group, job_handle dependency, Args&&... args);
            template<typename T, typename... Args>
            job_handle submit_to_group_with_dependencies(job_group group, std::span<const job_handle> dependencies, Args&&... args);

            // ------ lambdas and other type of callables for groups
            template<typename F, typename... Args>
            job_handle submit_to_group(F&& func, job_group group, Args&&... args);
            template<typename F, typename... Args>
            job_handle submit_to_group_with_dependency(F&& func, job_group group, job_handle dependency, Args&&... args);
            template<typename F, typename... Args>
            job_handle submit_to_group_with_dependencies(F&& func, job_group group, std::span<const job_handle> dependencies, Args&&... args);
        };
    
}


// #################################################################################################
// ----------------------------------------- JOBS MANAGER ----------------------------------------//
// #################################################################################################

template<typename T, typename... Args>
jman::job_handle jman::jobs_manager::submit(Args&&... args)
{
    return {};
}
template<typename T, typename... Args>
jman::job_handle jman::jobs_manager::submit_after(job_handle dependency, Args&&... args)
{
    return {};
}
template<typename T, typename... Args>
jman::job_handle jman::jobs_manager::submit_after(std::span<const job_handle> dependencies, Args&&... args)
{
    return {};
}

template<typename F, typename... Args>
jman::job_handle jman::jobs_manager::submit(F&& func, Args&&... args)
{
    return {};
}
template<typename F, typename... Args>
jman::job_handle jman::jobs_manager::submit_after(F&& func, job_handle dependency, Args&&... args)
{
    return {};
}
template<typename F, typename... Args>
jman::job_handle jman::jobs_manager::submit_after(F&& func, std::span<const job_handle> dependencies, Args&&... args)
{
    return {};
}







template<typename T, typename... Args>
jman::job_handle jman::jobs_manager::submit_to_group(job_group group, Args&&... args)
{
    return {};
}
template<typename T, typename... Args>
jman::job_handle jman::jobs_manager::submit_to_group_with_dependency(job_group group, job_handle dependency, Args&&... args)
{
    return {};
}
template<typename T, typename... Args>
jman::job_handle jman::jobs_manager::submit_to_group_with_dependencies(job_group group, std::span<const job_handle> dependencies, Args&&... args)
{
    return {};
}




template<typename F, typename... Args>
jman::job_handle jman::jobs_manager::submit_to_group(F&& func, job_group group, Args&&... args)
{
    return {};
}
template<typename F, typename... Args>
jman::job_handle jman::jobs_manager::submit_to_group_with_dependency(F&& func, job_group group, job_handle dependency, Args&&... args)
{
    return {};
}
template<typename F, typename... Args>
jman::job_handle jman::jobs_manager::submit_to_group_with_dependencies(F&& func, job_group group, std::span<const job_handle> dependencies, Args&&... args)
{
    return {};
}