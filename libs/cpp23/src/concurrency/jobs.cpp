#include <justaman/concurrency/jobs.hpp>

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <stop_token>
#include <vector>
#include <thread>
#include <new>

#ifdef __cpp_lib_hardware_interference_size
    static constexpr std::size_t CACHE_LINE_SIZE = std::hardware_constructive_interference_size;
#else
    static constexpr std::size_t CACHE_LINE_SIZE = 64 // fallback to 64
#endif

// #################################################################################################
// ------------------------------------------ JOB STRUCT ----------------------------------------//
// #################################################################################################

struct jman::__detail::jobs::job 
{
    job_function            func;
    job*                    parent;
    job_group               group;
    job_state               state;

    std::byte               __padding[CACHE_LINE_SIZE - 32];
};  // add padding to avoid false sharing problems when work stealing (workers invalidate the other workers cache lines when stealing)

// #################################################################################################
// ------------------------------------- JOBS_POOL CLASSD DECL. ----------------------------------//
// #################################################################################################

class job_pool 
{

};

// #################################################################################################
// -------------------------------------- WORKER CLASS DECL. -------------------------------------//
// #################################################################################################

class jman::__detail::jobs::worker 
{
    std::deque<job>                     m_tasks {};
    std::mutex                          m_mutex {};
    jobs_manager*                       m_manager = nullptr;
    /*
        its not generally safe to use raw "this" ptr to get the manager class but for this specific situation where this class
        will ALWAYS be destroyed first then there is no need to worry
    */
    std::jthread                        m_thread;
    std::uint32_t                       m_workerId;
    
    private:
        void operator()(std::stop_token); // worker loop

    public:
        worker(jobs_manager* manager, std::uint32_t id);
        ~worker() noexcept = default;

        worker(const worker& other) = delete;
        worker& operator=(const worker& other) = delete;

        worker(worker&& other) noexcept;
        worker& operator=(worker&& other) noexcept;

        JMAN_NO_DISCARD JMAN_FORCE_INLINE std::uint32_t id() const noexcept { return m_workerId; }
};

// #################################################################################################
// -------------------------------------- JOBS MANAGER IMPL --------------------------------------//
// #################################################################################################

struct jman::jobs_manager::impl
{
    std::vector<__detail::jobs::worker>                 workers {};
    std::atomic_bool                                    stop { false };

    static thread_local std::uint32_t          worker_index;
};
thread_local std::uint32_t jman::jobs_manager::impl::worker_index = 0; // leave 0 for main thread or other threads that dont belong to this manager

// #################################################################################################
// -------------------------------------- WORKER CLASS IMPL --------------------------------------//
// #################################################################################################

void jman::__detail::jobs::worker::operator()(std::stop_token token)
{
    /*
        while (!stop)
        {
            if (TryPopLocal(job))
            {
                Execute(job);
                continue;
            }

            if (TrySteal(job))
            {
                Execute(job);
                continue;
            }

            WaitForWork();
        }
    */
}

jman::__detail::jobs::worker::worker(jobs_manager* manager, std::uint32_t id)
    : m_manager(manager), m_workerId(id)
{   
    m_thread = std::jthread([this, id](std::stop_token tk)
    { 
        m_manager->m_impl->worker_index = id;
        operator()(tk); 
    });
}

// worker::~worker() noexcept
// {
    
// }

jman::__detail::jobs::worker::worker(worker&& other) noexcept
{
    std::lock_guard<std::mutex> lock {other.m_mutex};

    m_manager = other.m_manager;
    m_workerId = other.m_workerId;
    m_tasks = std::move(other.m_tasks);
    m_thread = std::move(other.m_thread);
    // mutex already created
}
jman::__detail::jobs::worker& jman::__detail::jobs::worker::operator=(worker&& other) noexcept
{
    if (this != &other)
    {
        std::lock_guard<std::mutex> lock {other.m_mutex};

        m_manager = other.m_manager;
        m_workerId = other.m_workerId;
        m_tasks = std::move(other.m_tasks);
        m_thread = std::move(other.m_thread);
        // mutex already created
    }

    return *this;
}


// #################################################################################################
// ------------------------------------- JOBS MANAGER PRIVATE ------------------------------------//
// #################################################################################################


// #################################################################################################
// ---------------------------- JOBS_MANAGER CONSTRUCTORS/DESTRUCTORS ----------------------------//
// #################################################################################################

jman::jobs_manager::jobs_manager(std::size_t workerCount)
{
    if (workerCount == 0)
        workerCount = std::thread::hardware_concurrency();
    if (workerCount == 0)
        workerCount = 1; // hardware_concurrency() can return 0 on some dumbass platforms

    m_impl->workers.reserve(workerCount);
    for (std::uint32_t i = 0; i < workerCount; ++i)
        m_impl->workers.emplace_back(this, i);
}

jman::jobs_manager::~jobs_manager() noexcept
{
    shutdown();
}

// #################################################################################################
// ------------------------------------- JOBS_MANAGER METHODS ------------------------------------//
// #################################################################################################

void jman::jobs_manager::shutdown()
{
    m_impl->stop.store(true, std::memory_order_release);
}




JMAN_NO_DISCARD std::uint32_t jman::jobs_manager::workers() const noexcept
{
    return m_impl->workers.size();
}
JMAN_NO_DISCARD bool jman::jobs_manager::is_worker_thread() const noexcept
{
    return m_impl->worker_index != 0;
}
JMAN_NO_DISCARD std::uint32_t jman::jobs_manager::current_worker_index() const noexcept
{
    return m_impl->worker_index;
}




JMAN_NO_DISCARD bool jman::jobs_manager::exists(job_handle handle) const noexcept
{
    return {};
}
JMAN_NO_DISCARD bool jman::jobs_manager::cancel(job_handle job)
{
    return {};
}
JMAN_NO_DISCARD jman::job_state jman::jobs_manager::state(job_handle job) const noexcept
{
    return {};
}
JMAN_NO_DISCARD bool jman::jobs_manager::completed(job_handle job) const noexcept
{
    return {};
}


bool jman::jobs_manager::add_dependency(job_handle job, job_handle dependency)
{
    return {};
}
bool jman::jobs_manager::add_dependencies(job_handle job, std::span<const job_handle> dependencies)
{
    return {};
}
void jman::jobs_manager::wait(job_handle job)
{

}
void jman::jobs_manager::wait(std::span<const job_handle> jobs)
{

}


jman::job_group jman::jobs_manager::create_group()
{
    return {};
}
void jman::jobs_manager::destroy_group(job_group group)
{

}

void jman::jobs_manager::add_to_group(job_group group, job_handle job)
{

}
JMAN_NO_DISCARD bool jman::jobs_manager::group_completed(job_group group) const noexcept
{
    return {};
}
void jman::jobs_manager::wait_group(job_group group)
{
    
}