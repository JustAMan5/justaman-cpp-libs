#pragma once

#include "justaman/meta/type_traits.hpp"
#include "justaman/utils/attributes.h"
#include <functional>
#include <future>

/**
    @brief 
        use for when needing to push long lived tasks regulary
*/

namespace jman 
{

    class thread_pool 
    {
        struct impl;
        std::unique_ptr<impl>               m_impl;

        public:   

        private:
            void                            push(std::move_only_function<void(std::stop_token)>&& task, std::size_t priority);

        public:
            explicit thread_pool(std::uint32_t workers = 0);
            ~thread_pool() noexcept;

            thread_pool(const thread_pool&) = delete;    
            thread_pool& operator=(const thread_pool&) = delete;

            thread_pool(thread_pool&&) noexcept = delete;    
            thread_pool& operator=(thread_pool&&) noexcept = delete;

            template <typename F, typename... Args>
            requires jman::is_callable_v<F, std::stop_token, Args...>
            auto                            submit(F&& func, std::size_t priority = 0, Args&&... args) -> std::future<std::invoke_result_t<F, std::stop_token, Args...>>
            {
                using R = std::invoke_result_t<F, std::stop_token, Args...>; // gets the result type for F depending on Args... types

                std::packaged_task<R(std::stop_token tk)> task{ // create a promise and wrap it around the func 
                    [func = std::forward<F>(func), ...args = std::forward<Args>(args)](std::stop_token tk) mutable {
                        return std::invoke(std::move(func), tk, std::move(args)...);
                    }};
                std::future<R> fut = task.get_future(); // grabs the future before moving the task
                auto wrapper_func = [task = std::move(task)](std::stop_token tk) mutable { std::move(task)(tk); }; // move the task inside the lambda capture

                push(std::move(wrapper_func), priority);
                return fut;
            }

            void                            wait_all();
            void                            shutdown() noexcept;

            JMAN_NO_DISCARD std::uint32_t   workers() const noexcept;
            JMAN_NO_DISCARD std::size_t     pending() const noexcept;
            JMAN_NO_DISCARD std::size_t     working() const noexcept;
    };
    
}