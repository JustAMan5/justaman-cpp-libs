#include <justaman/concurrency/thread_pool.hpp>
#include <atomic>
#include <functional>
#include <stop_token>
#include <thread>
#include <condition_variable>
#include <justaman/containers/priority_queue.hpp>
#include <mutex>
// #include "lockfree/queue.hpp"

// ################################################################################################
// ----------------------------------------- PRIVATE IMPL ---------------------------------------//
// ################################################################################################

struct jman::thread_pool::impl 
{
    std::vector<std::jthread>                  workers {};
    jman::priority_queue
        <std::move_only_function<void(std::stop_token)>>      tasks {};
    
    std::atomic_bool                           stop { false };
    std::atomic_size_t                         running_tasks   { 0 };   // tasks currently running
    std::condition_variable                    idle_cv {};                 // notified when running_tasks hits 0
    std::condition_variable                    submited_or_stop_cv {};     // notified when a task is pushed or shutdown is called    
    mutable std::mutex                         mutex {};

    void operator()(std::stop_token tk)
    {
        while (true) {
            std::move_only_function<void(std::stop_token)> task;

            {
                std::unique_lock<std::mutex> lock(mutex);

                submited_or_stop_cv.wait(lock, [this] {
                    return stop || !tasks.empty();
                });

                if (stop && tasks.empty()) return;

                task = std::move(tasks.peek());
                tasks.pop();
                running_tasks.fetch_add(1, std::memory_order_release);
            }

            task(tk); // run outside the lock so other threads can pick up work
            // no exception catching since task is a std::packaged_task<> type and packaged task already does that

            {
                std::unique_lock<std::mutex> lock(mutex);
                running_tasks.fetch_sub(1, std::memory_order_release);
                idle_cv.notify_all(); // wake anyone blocked in wait_all()
            }

        }
    }
};

// ################################################################################################
// --------------------------------------- PRIVATE METHODS --------------------------------------//
// ################################################################################################

void jman::thread_pool::push(std::move_only_function<void(std::stop_token)>&& task, std::size_t priority)
{
    if (m_impl->stop)
        throw std::runtime_error("justaman::thread_pool::submit failed: submit() called after shutdown()");

    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->tasks.push(std::move(task), priority);
    m_impl->submited_or_stop_cv.notify_one();
}


// ################################################################################################
// ------------------------------ PUBLIC CONSTRUCTORS / DESTRUCTORS ----------------------------//
// ################################################################################################

jman::thread_pool::thread_pool(std::uint32_t thread_count)
    : m_impl(std::make_unique<impl>()) 
{

    if (thread_count == 0)
        thread_count = std::thread::hardware_concurrency();
    if (thread_count == 0)
        thread_count = 1; // hardware_concurrency() can return 0 on some dumbass platforms

    m_impl->workers.reserve(thread_count);
    for (std::uint32_t i = 0; i < thread_count; ++i)
        m_impl->workers.emplace_back(std::ref(*m_impl));
}

jman::thread_pool::~thread_pool() noexcept 
{
    shutdown();
}


// ################################################################################################
// ---------------------------------------- PUBLIC METHODS --------------------------------------//
// ################################################################################################

void jman::thread_pool::wait_all() 
{
    std::unique_lock<std::mutex> lock(m_impl->mutex);
    m_impl->idle_cv.wait(lock, [this] {
        return m_impl->tasks.empty() && m_impl->running_tasks == 0;
    });
}

void jman::thread_pool::shutdown() noexcept 
{
    m_impl->stop.store(true, std::memory_order_release);
    m_impl->submited_or_stop_cv.notify_all();
}

std::uint32_t jman::thread_pool::workers() const noexcept 
{
    return m_impl->workers.size();
}

std::size_t jman::thread_pool::pending() const noexcept 
{
    std::lock_guard<std::mutex> lock {m_impl->mutex};
    return m_impl->tasks.size();
}

std::size_t jman::thread_pool::working() const noexcept 
{
    return m_impl->running_tasks.load(std::memory_order_acquire);
}