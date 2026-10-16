#include <atomic>
#include <chrono>
#include <concepts>
#include <cstdlib>
#include <iostream>
#include <latch>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <seamwork/studies/structured_delayed_work.hpp>

namespace sw = seamwork::studies::structured_delayed_work;
using namespace std::chrono_literals;

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

struct throwing_callback {
    void operator()(std::stop_token)
    {
    }
};

struct noexcept_callback {
    void operator()(std::stop_token) noexcept
    {
    }
};

template <class Task>
concept startable_delayed_task = requires(Task&& task) {
    sw::delayed_task::start(0ms, std::forward<Task>(task));
};

static_assert(!std::copy_constructible<sw::delayed_task>);
static_assert(std::move_constructible<sw::delayed_task>);
static_assert(!std::is_move_assignable_v<sw::delayed_task>);
static_assert(!startable_delayed_task<throwing_callback>);
static_assert(startable_delayed_task<noexcept_callback>);

void test_zero_delay_executes_owned_callback()
{
    std::atomic_bool called{false};

    auto task = sw::delayed_task::start(
        0ms,
        [&called](std::stop_token) noexcept {
            called.store(true, std::memory_order_release);
        }
    );

    task.join();

    require(called.load(std::memory_order_acquire), "zero-delay task should execute");
    require(!task.joinable(), "joined task should no longer own a joinable thread");
}

void test_cancellation_before_deadline_prevents_callback()
{
    std::atomic_bool called{false};

    auto task = sw::delayed_task::start(
        std::chrono::hours{1},
        [&called](std::stop_token) noexcept {
            called.store(true, std::memory_order_release);
        }
    );

    require(task.request_stop(), "pending task should accept first stop request");
    task.join();

    require(!called.load(std::memory_order_acquire), "cancelled pending task should not run");
    require(task.stop_requested(), "owner should expose requested stop state");
}

void test_stop_racing_with_wait_setup()
{
    std::atomic<unsigned> called{0};

    // Race immediate cancellation with worker setup, then also give the
    // worker a chance to enter its wait on alternating iterations.
    for (int attempt = 0; attempt < 256; ++attempt) {
        auto task = sw::delayed_task::start(
            std::chrono::hours{1},
            [&called](std::stop_token) noexcept {
                called.fetch_add(1, std::memory_order_relaxed);
            }
        );

        if (attempt % 2 != 0) {
            std::this_thread::yield();
        }

        require(task.request_stop(), "pending task should accept a stop request");
        task.join();
    }

    require(
        called.load(std::memory_order_relaxed) == 0,
        "a cancelled long-delay task must never execute"
    );
}

void test_destruction_cancels_and_joins_pending_work()
{
    std::atomic_bool called{false};

    {
        auto task = sw::delayed_task::start(
            std::chrono::hours{1},
            [&called](std::stop_token) noexcept {
                called.store(true, std::memory_order_release);
            }
        );

        require(task.joinable(), "started task should own a joinable worker");
    }

    require(
        !called.load(std::memory_order_acquire),
        "owner destruction should cancel pending work before returning"
    );
}

void test_move_transfers_worker_ownership()
{
    std::atomic_bool called{false};

    auto first = sw::delayed_task::start(
        std::chrono::hours{1},
        [&called](std::stop_token) noexcept {
            called.store(true, std::memory_order_release);
        }
    );

    auto second = std::move(first);

    // delayed_task defines its moved-from state through std::jthread ownership.
    // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
    require(!first.joinable(), "moved-from task should release worker ownership");
    require(second.joinable(), "moved-to task should own worker");

    static_cast<void>(second.request_stop());
    second.join();

    require(!called.load(std::memory_order_acquire), "moved task should still cancel safely");
}

void test_running_callback_observes_stop_token()
{
    std::latch started{1};
    std::atomic_bool observed_stop{false};

    auto task = sw::delayed_task::start(
        0ms,
        [&started, &observed_stop](std::stop_token stop) noexcept {
            started.count_down();

            while (!stop.stop_requested()) {
                std::this_thread::yield();
            }

            observed_stop.store(true, std::memory_order_release);
        }
    );

    started.wait();
    static_cast<void>(task.request_stop());
    task.join();

    require(
        observed_stop.load(std::memory_order_acquire),
        "running callback should receive cooperative stop request"
    );
}

} // namespace

int main()
{
    try {
        test_zero_delay_executes_owned_callback();
        test_cancellation_before_deadline_prevents_callback();
        test_stop_racing_with_wait_setup();
        test_destruction_cancels_and_joins_pending_work();
        test_move_transfers_worker_ownership();
        test_running_callback_observes_stop_token();
    } catch (const std::exception& error) {
        std::cerr << "structured-delayed-work study failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
