#include <atomic>
#include <chrono>
#include <iostream>

#include <seamwork/studies/structured_delayed_work.hpp>

namespace sw = seamwork::studies::structured_delayed_work;
using namespace std::chrono_literals;

int main()
{
    std::atomic_bool refreshed{false};

    auto refresh = sw::delayed_task::start(
        25ms,
        [&refreshed](std::stop_token stop) noexcept {
            if (!stop.stop_requested()) {
                refreshed.store(true, std::memory_order_release);
            }
        }
    );

    refresh.join();

    std::cout << "refreshed: "
              << (refreshed.load(std::memory_order_acquire) ? "yes" : "no")
              << '\n';
}
