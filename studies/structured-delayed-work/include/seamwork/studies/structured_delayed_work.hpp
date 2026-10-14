#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stop_token>
#include <thread>
#include <type_traits>
#include <utility>

namespace seamwork::studies::structured_delayed_work {

class delayed_task
{
public:
    delayed_task(const delayed_task&) = delete;
    auto operator=(const delayed_task&) -> delayed_task& = delete;

    delayed_task(delayed_task&&) noexcept = default;
    auto operator=(delayed_task&&) -> delayed_task& = delete;

    ~delayed_task() = default;

    template <class Task>
        requires std::is_nothrow_invocable_v<std::decay_t<Task>&, std::stop_token>
    [[nodiscard]] static auto start(
        std::chrono::milliseconds delay,
        Task&& task
    ) -> delayed_task
    {
        using task_type = std::decay_t<Task>;

        return delayed_task{std::jthread{
            [delay, work = task_type{std::forward<Task>(task)}](
                std::stop_token stop
            ) mutable noexcept {
                std::mutex mutex;
                std::condition_variable_any wake;
                std::unique_lock lock{mutex};

                // The stop-aware wait registers cancellation without a gap
                // between checking the token and entering the wait.
                static_cast<void>(wake.wait_for(
                    lock,
                    stop,
                    delay,
                    []() noexcept { return false; }
                ));

                if (stop.stop_requested()) {
                    return;
                }

                lock.unlock();
                work(stop);
            }
        }};
    }

    [[nodiscard]] auto request_stop() noexcept -> bool
    {
        return worker_.request_stop();
    }

    [[nodiscard]] auto stop_requested() const noexcept -> bool
    {
        return worker_.get_stop_token().stop_requested();
    }

    [[nodiscard]] auto joinable() const noexcept -> bool
    {
        return worker_.joinable();
    }

    void join()
    {
        if (worker_.joinable()) {
            worker_.join();
        }
    }

private:
    explicit delayed_task(std::jthread worker) noexcept
        : worker_(std::move(worker))
    {
    }

    std::jthread worker_;
};

} // namespace seamwork::studies::structured_delayed_work
